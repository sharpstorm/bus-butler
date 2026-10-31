#include "DisplayConfigApi.h"

#include "../../Config/ConfigManager.h"
#include "../HttpConstants.h"

static String toHhmm(uint16_t minutes) {
  char buf[6];
  snprintf(buf, sizeof(buf), "%02u:%02u", minutes / 60, minutes % 60);
  return buf;
}

// "HH:MM" -> minutes after midnight, or -1
static int fromHhmm(const String& value) {
  int colon = value.indexOf(':');
  if (colon < 1) return -1;
  int h = value.substring(0, colon).toInt();
  int m = value.substring(colon + 1).toInt();
  if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
  return h * 60 + m;
}

// Brightness is 0-255 on the device and a percentage in the portal.
static uint8_t toPercent(uint8_t level) { return (level * 100 + 127) / 255; }
static uint8_t fromPercent(int percent) {
  return (constrain(percent, 0, 100) * 255 + 50) / 100;
}

void DisplayConfigApi::getDisplay(AsyncWebServerRequest* request) {
  DisplayConfig config = configManager.getDisplayConfig();
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject root = response->getRoot().to<JsonObject>();
  root["brightness"] = toPercent(config.brightness);
  root["idleDimEnabled"] = config.idleDimEnabled;
  root["idleDimMinutes"] = config.idleDimMinutes;
  root["idleDimLevel"] = toPercent(config.idleDimLevel);
  root["nightEnabled"] = config.nightEnabled;
  root["nightTheme"] = config.nightTheme;
  root["nightStart"] = toHhmm(config.nightStart);
  root["nightEnd"] = toHhmm(config.nightEnd);
  root["nightLevel"] = toPercent(config.nightLevel);
  response->setLength();
  request->send(response);
}

void DisplayConfigApi::saveDisplay(AsyncWebServerRequest* request,
                                   JsonVariant& body) {
  DisplayConfig config = configManager.getDisplayConfig();

  if (!body["brightness"].isNull()) {
    // Never let the portal turn the screen fully dark during the day
    config.brightness = max<uint8_t>(fromPercent(body["brightness"]), 8);
  }
  if (!body["idleDimEnabled"].isNull()) {
    config.idleDimEnabled = body["idleDimEnabled"];
  }
  if (!body["idleDimMinutes"].isNull()) {
    config.idleDimMinutes = constrain((int)body["idleDimMinutes"], 1, 120);
  }
  if (!body["idleDimLevel"].isNull()) {
    config.idleDimLevel = fromPercent(body["idleDimLevel"]);
  }
  if (!body["nightEnabled"].isNull()) {
    config.nightEnabled = body["nightEnabled"];
  }
  if (!body["nightTheme"].isNull()) {
    config.nightTheme = body["nightTheme"];
  }
  if (!body["nightStart"].isNull()) {
    int minutes = fromHhmm(body["nightStart"] | "");
    if (minutes < 0) {
      request->send(400, MimeType::JSON, "{\"error\":\"nightStart is HH:MM\"}");
      return;
    }
    config.nightStart = minutes;
  }
  if (!body["nightEnd"].isNull()) {
    int minutes = fromHhmm(body["nightEnd"] | "");
    if (minutes < 0) {
      request->send(400, MimeType::JSON, "{\"error\":\"nightEnd is HH:MM\"}");
      return;
    }
    config.nightEnd = minutes;
  }
  if (!body["nightLevel"].isNull()) {
    config.nightLevel = fromPercent(body["nightLevel"]);
  }

  // The display task picks this up from the config revision.
  configManager.saveDisplayConfig(config);
  request->send(200, MimeType::JSON, "{\"success\":true}");
}
