#include "NotifyApi.h"

#include <esp_random.h>

#include "../Config/ConfigManager.h"
#include "../WebConfigServer/HttpConstants.h"
#include "NotifyStore.h"

namespace NotifyApi {

// The token is read from flash once and kept here; the portal updates both.
static SemaphoreHandle_t tokenMutex = nullptr;
static String cachedToken;
static bool tokenLoaded = false;

static String currentToken() {
  if (tokenMutex == nullptr) tokenMutex = xSemaphoreCreateMutex();
  xSemaphoreTake(tokenMutex, portMAX_DELAY);
  if (!tokenLoaded) {
    cachedToken = configManager.getNotifyToken();
    tokenLoaded = true;
  }
  String token = cachedToken;
  xSemaphoreGive(tokenMutex);
  return token;
}

static void setToken(const String& token) {
  configManager.saveNotifyToken(token);
  if (tokenMutex == nullptr) tokenMutex = xSemaphoreCreateMutex();
  xSemaphoreTake(tokenMutex, portMAX_DELAY);
  cachedToken = token;
  tokenLoaded = true;
  xSemaphoreGive(tokenMutex);
}

// Same time whatever the input, so the token can't be guessed byte by byte.
static bool sameSecret(const String& a, const String& b) {
  size_t n = max(a.length(), b.length());
  uint8_t diff = a.length() != b.length();
  for (size_t i = 0; i < n; i++) {
    uint8_t x = i < a.length() ? a[i] : 0;
    uint8_t y = i < b.length() ? b[i] : 0;
    diff |= x ^ y;
  }
  return diff == 0;
}

static void sendError(AsyncWebServerRequest* request, int code,
                      const char* message) {
  JsonDocument doc;
  doc["error"] = message;
  String out;
  serializeJson(doc, out);
  request->send(code, MimeType::JSON, out);
}

// Returns 0 if the request may push, else the HTTP status to answer.
static int authCode(AsyncWebServerRequest* request) {
  String token = currentToken();
  if (token.length() == 0) return 403;
  const String& header = request->header("Authorization");
  if (!header.startsWith("Bearer ")) return 401;
  return sameSecret(header.substring(7), token) ? 0 : 401;
}

static bool authorize(AsyncWebServerRequest* request) {
  int code = authCode(request);
  if (code == 403) {
    sendError(request, 403,
              "Notifications are off: set a token in the device's portal");
  } else if (code != 0) {
    sendError(request, 401, "Missing or wrong bearer token");
  }
  return code == 0;
}

static bool validId(const String& id) {
  if (id.length() == 0 || id.length() > NOTIFY_ID_MAX) return false;
  for (size_t i = 0; i < id.length(); i++) {
    char c = id[i];
    if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-' ||
          c == ':')) {
      return false;
    }
  }
  return true;
}

static String clip(const String& s, size_t max) {
  if (s.length() <= max) return s;
  // Don't cut a UTF-8 sequence in half
  size_t end = max;
  while (end > 0 && ((uint8_t)s[end] & 0xC0) == 0x80) end--;
  return s.substring(0, end);
}

// Sets out from a string field if the request has it.
static bool take(JsonVariant body, const char* key, String& out, size_t max) {
  if (body[key].isNull()) return false;
  out = clip(body[key] | "", max);
  return true;
}

// Builds a notification from JSON. Returns an error message, or "" if fine.
// With "merge": true, fields the request leaves out keep their current
// values (an update that only changes a badge or the buttons).
static String parse(JsonVariant body, Notification& n, bool& keepTimer) {
  if (!body.is<JsonObject>()) return "Expected a JSON object";
  String id = body["id"] | "";
  if (!validId(id)) {
    return "id must be 1-32 characters: letters, digits, . _ - :";
  }
  Notification old;
  bool exists = notifyStore.get(id, old);
  bool merge = (body["merge"] | false) && exists;
  if (merge) {
    n = old;
  } else {
    n = Notification();
    n.id = id;
  }
  String timer = body["timer"] | "restart";
  if (timer != "restart" && timer != "keep") return "timer must be keep or restart";
  keepTimer = timer == "keep";

  if (!body["style"].isNull() || !merge) {
    String style = body["style"] | "toast";
    if (!parseNotifyStyle(style, n.style)) {
      return "style must be full, banner or toast";
    }
  }
  bool styleChanged = exists && old.style != n.style;
  take(body, "title", n.title, NOTIFY_TITLE_MAX);
  take(body, "body", n.body, NOTIFY_BODY_MAX);
  if (n.title.length() == 0 && n.body.length() == 0) {
    return "Give a title or a body";
  }
  take(body, "icon", n.icon, 16);
  take(body, "accent", n.accent, 12);
  take(body, "source", n.source, 24);
  take(body, "badge", n.badge, NOTIFY_BADGE_MAX);
  take(body, "badgeAccent", n.badgeAccent, 12);
  if (!body["priority"].isNull() || !merge) {
    n.priority = constrain(body["priority"] | 1, 0, 3);
  }

  if (!body["timeout"].isNull() || !merge || styleChanged) {
    int defaultTimeout = n.style == NotifyStyle::Full    ? NOTIFY_FULL_TIMEOUT_S
                         : n.style == NotifyStyle::Toast ? NOTIFY_TOAST_TIMEOUT_S
                                                         : 0;
    long timeout = body["timeout"] | (long)defaultTimeout;
    if (n.style == NotifyStyle::Toast) {
      timeout = constrain(timeout, 1L, 30L);
    } else {
      timeout = constrain(timeout, 0L, 86400L);
    }
    n.timeoutMs = (uint32_t)timeout * 1000;
  }
  if (!body["wake"].isNull() || !merge) {
    n.wake = body["wake"] | (n.style == NotifyStyle::Full);
  }

  String image = body["image"] | "";
  if (n.style != NotifyStyle::Full) {
    n.image = NotifyImage::None;
    n.jpeg.reset();
  } else if (image == "pending") {
    n.image = NotifyImage::Pending;
    if (exists) n.jpeg = old.jpeg;  // shown with "Updating" meanwhile
  } else if (image == "unavailable") {
    n.image = NotifyImage::Unavailable;
    n.jpeg.reset();
  } else if (image == "none") {
    n.image = NotifyImage::None;
    n.jpeg.reset();
  } else if (image.length() == 0) {
    // An update that doesn't mention the picture keeps it
    if (exists && old.style == NotifyStyle::Full) {
      n.image = old.image;
      n.jpeg = old.jpeg;
    }
  } else {
    return "image must be pending, none or unavailable";
  }

  if (!body["actions"].isNull() || !merge) {
    n.actions.clear();
    JsonArray actions = body["actions"].as<JsonArray>();
    for (JsonObject a : actions) {
      if (n.actions.size() >= NOTIFY_MAX_ACTIONS) break;
      NotifyAction action;
      action.label = clip(a["label"] | "", NOTIFY_ACTION_LABEL_MAX);
      action.url = a["url"] | "";
      action.method = a["method"] | "POST";
      action.method.toUpperCase();
      action.icon = clip(a["icon"] | "", 16);
      action.accent = clip(a["accent"] | "", 12);
      if (action.label.length() == 0) return "Every action needs a label";
      if (!action.url.startsWith("http://") ||
          action.url.length() > NOTIFY_URL_MAX) {
        return "Action urls must be http:// and at most 300 characters";
      }
      if (action.method != "GET" && action.method != "POST") {
        return "Action method must be GET or POST";
      }
      n.actions.push_back(action);
    }
  }
  return "";
}

static void handlePush(AsyncWebServerRequest* request, JsonVariant& body) {
  if (!authorize(request)) return;
  Notification n;
  bool keepTimer = false;
  String error = parse(body, n, keepTimer);
  if (error.length()) {
    sendError(request, 400, error.c_str());
    return;
  }
  notifyStore.upsert(n, keepTimer);
  JsonDocument doc;
  doc["ok"] = true;
  doc["id"] = n.id;
  doc["style"] = notifyStyleName(n.style);
  String out;
  serializeJson(doc, out);
  request->send(200, MimeType::JSON, out);
}

// ---- Picture upload ----------------------------------------------------------
// The body arrives in chunks on the web server task; it is gathered in PSRAM
// (request->_tempObject, freed by the library if we don't take it).

static void imageBody(AsyncWebServerRequest* request, uint8_t* data,
                      size_t len, size_t index, size_t total) {
  if (index == 0) {
    int code = authCode(request);
    if (code != 0) {
      request->setAttribute("err", (long)code);
      return;
    }
    if (total == 0 || total > NOTIFY_MAX_IMAGE_BYTES) {
      request->setAttribute("err", 413L);
      return;
    }
    if (len < 2 || data[0] != 0xFF || data[1] != 0xD8) {
      request->setAttribute("err", 415L);
      return;
    }
    request->_tempObject = ps_malloc(total);
    if (request->_tempObject == nullptr) {
      request->setAttribute("err", 507L);
      return;
    }
  }
  if (request->_tempObject == nullptr || index + len > total) return;
  memcpy((uint8_t*)request->_tempObject + index, data, len);
  if (index + len == total) request->setAttribute("done", true);
}

static void imageRequest(AsyncWebServerRequest* request) {
  long err = request->getAttribute("err", 0L);
  if (err == 401 || err == 403) {
    authorize(request);  // answers with the right message
    return;
  }
  if (err == 413) {
    sendError(request, 413, "Send a JPEG of at most 160 KB");
    return;
  }
  if (err == 415) {
    sendError(request, 415, "The body must be a JPEG");
    return;
  }
  if (err == 507) {
    sendError(request, 507, "Out of memory for the picture");
    return;
  }
  if (!authorize(request)) return;  // e.g. an empty body never reached imageBody
  if (!request->getAttribute("done", false) ||
      request->_tempObject == nullptr) {
    sendError(request, 400, "Send the JPEG as the request body");
    return;
  }
  const AsyncWebParameter* id = request->getParam("id");
  if (id == nullptr) {
    sendError(request, 400, "Add ?id=<notification id>");
    return;
  }
  const AsyncWebParameter* timer = request->getParam("timer");
  bool keepTimer = timer != nullptr && timer->value() == "keep";
  uint8_t* data = (uint8_t*)request->_tempObject;
  request->_tempObject = nullptr;  // the store owns it now
  if (!notifyStore.attachImage(id->value(), data, request->contentLength(),
                               keepTimer)) {
    sendError(request, 404, "No notification with that id");
    return;
  }
  request->send(200, MimeType::JSON, "{\"ok\":true}");
}

// ---- Dismiss and list --------------------------------------------------------

static void handleDismiss(AsyncWebServerRequest* request) {
  if (!authorize(request)) return;
  const AsyncWebParameter* id = request->getParam("id");
  if (id == nullptr) {
    sendError(request, 400, "Add ?id=<notification id> (or id=*)");
    return;
  }
  if (id->value() == "*") {
    notifyStore.clear();
  } else if (!notifyStore.remove(id->value())) {
    sendError(request, 404, "No notification with that id");
    return;
  }
  request->send(200, MimeType::JSON, "{\"ok\":true}");
}

static const char* imageName(NotifyImage image) {
  switch (image) {
    case NotifyImage::Pending:
      return "pending";
    case NotifyImage::Ready:
      return "ready";
    case NotifyImage::Unavailable:
      return "unavailable";
    default:
      return "none";
  }
}

static void handleList(AsyncWebServerRequest* request) {
  if (!authorize(request)) return;
  AsyncJsonResponse* response = new AsyncJsonResponse(true);
  JsonArray root = response->getRoot().to<JsonArray>();
  uint32_t now = millis();
  for (const Notification& n : notifyStore.list()) {
    JsonObject o = root.add<JsonObject>();
    o["id"] = n.id;
    o["style"] = notifyStyleName(n.style);
    o["title"] = n.title;
    o["priority"] = n.priority;
    o["image"] = imageName(n.image);
    o["age"] = (now - n.createdMs) / 1000;
    if (n.deadlineMs) o["expiresIn"] = (int32_t)(n.deadlineMs - now) / 1000;
    if (n.badge.length()) o["badge"] = n.badge;
    o["imageRevision"] = n.imageRevision;
  }
  response->setLength();
  request->send(response);
}

// ---- Portal ------------------------------------------------------------------

static void getConfig(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject root = response->getRoot().to<JsonObject>();
  root["tokenSet"] = currentToken().length() > 0;
  root["full"] = notifyStore.count(NotifyStyle::Full);
  root["banner"] = notifyStore.count(NotifyStyle::Banner);
  root["toast"] = notifyStore.count(NotifyStyle::Toast);
  response->setLength();
  request->send(response);
}

static void saveConfig(AsyncWebServerRequest* request, JsonVariant& body) {
  String token = body["token"] | "";
  token.trim();
  if (token.length() && (token.length() < 16 || token.length() > 64)) {
    sendError(request, 400, "The token must be 16 to 64 characters");
    return;
  }
  for (size_t i = 0; i < token.length(); i++) {
    if (token[i] <= ' ' || token[i] > '~') {
      sendError(request, 400, "Use letters, digits and symbols, no spaces");
      return;
    }
  }
  setToken(token);
  request->send(200, MimeType::JSON, "{\"success\":true}");
}

static void generateToken(AsyncWebServerRequest* request) {
  char token[33];
  for (int i = 0; i < 16; i++) {
    snprintf(token + i * 2, 3, "%02x", (uint8_t)(esp_random() & 0xFF));
  }
  setToken(token);
  JsonDocument doc;
  doc["token"] = token;
  String out;
  serializeJson(doc, out);
  request->send(200, MimeType::JSON, out);
}

static void sendTest(AsyncWebServerRequest* request, JsonVariant& body) {
  Notification n;
  String style = body["style"] | "toast";
  if (!parseNotifyStyle(style, n.style)) n.style = NotifyStyle::Toast;
  n.id = String("portal-test-") + style;
  n.source = "Web portal";
  n.icon = "bell";
  n.accent = "magenta";
  switch (n.style) {
    case NotifyStyle::Full:
      n.title = "Test notification";
      n.body = "Full-screen notifications close by themselves after their "
               "timeout, when dismissed over HTTP, or when you tap Close.";
      n.timeoutMs = 20000;
      n.wake = true;
      break;
    case NotifyStyle::Banner:
      n.title = "Test banner";
      n.body = "Banners stay on the Home page until dismissed (this one: 1 min)";
      n.timeoutMs = 60000;
      break;
    default:
      n.title = "Test toast";
      n.body = "Tap or swipe to dismiss";
      n.timeoutMs = 6000;
      break;
  }
  notifyStore.upsert(n);
  request->send(200, MimeType::JSON, "{\"success\":true}");
}

void registerRoutes(AsyncWebServer& server) {
  using M = AsyncURIMatcher;
  server.on(M::exact("/api/notify/image"), HTTP_PUT, imageRequest, nullptr,
            imageBody);
  server.on(M::exact("/api/notify/dismiss"), HTTP_POST, handleDismiss);
  server.on(M::exact("/api/notify"), HTTP_POST, handlePush);
  server.on(M::exact("/api/notify"), HTTP_DELETE, handleDismiss);
  server.on(M::exact("/api/notify"), HTTP_GET, handleList);

  server.on(M::exact("/api/config/notify/generate"), HTTP_POST, generateToken);
  server.on(M::exact("/api/config/notify/test"), HTTP_POST, sendTest);
  server.on(M::exact("/api/config/notify"), HTTP_GET, getConfig);
  server.on(M::exact("/api/config/notify"), HTTP_POST, saveConfig);
}

}  // namespace NotifyApi
