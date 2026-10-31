#include "StatusApi.h"

#include "../../Data/DataStore.h"
#include "../../Display/DisplayPower.h"
#include "../../Ui/Anim.h"
#include "../../States/Status/StatusPageState.h"
#include "../../Util/Time.h"
#include "../HttpConstants.h"

static void writeSource(JsonObject out, const SourceStatus& s) {
  out["attempted"] = s.attempted;
  out["ok"] = s.ok;
  out["error"] = s.lastError;
  if (s.lastSuccessMs > 0) {
    out["lastSuccessAgo"] = (millis() - s.lastSuccessMs) / 1000;
  } else {
    out["lastSuccessAgo"] = nullptr;
  }
}

void StatusApi::getStatus(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject root = response->getRoot().to<JsonObject>();

  root["firmware"] = FIRMWARE_VERSION;
  root["uptime"] = millis() / 1000;
  root["clockSynced"] = TimeUtil::isSynced();
  root["freeHeap"] = ESP.getFreeHeap();
  root["freePsram"] = ESP.getFreePsram();

  JsonObject wifi = root["wifi"].to<JsonObject>();
  wifi["ssid"] = WiFi.SSID();
  wifi["rssi"] = WiFi.RSSI();
  wifi["ip"] = WiFi.localIP().toString();

  JsonObject display = root["display"].to<JsonObject>();
  display["level"] = (displayPower.getAppliedLevel() * 100 + 127) / 255;
  display["idleDimmed"] = displayPower.isIdleDimmed();
  display["nightActive"] = displayPower.isNightActive();

  JsonObject animation = root["animation"].to<JsonObject>();
  animation["fps"] = anim::stats().fps();
  animation["frames"] = anim::stats().frames;
  animation["flushMs"] = anim::stats().flushUs / 1000.0f;

  dataStore.lock();
  root["activeStop"] = dataStore.getActiveStop();
  JsonObject sources = root["sources"].to<JsonObject>();
  writeSource(sources["bus"].to<JsonObject>(), dataStore.busStatus);
  writeSource(sources["mrt"].to<JsonObject>(), dataStore.trainStatus);
  writeSource(sources["forecast"].to<JsonObject>(), dataStore.weatherStatus);
  writeSource(sources["readings"].to<JsonObject>(), dataStore.extrasStatus);
  dataStore.unlock();

  response->setLength();
  request->send(response);
}

void StatusApi::getMrt(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject root = response->getRoot().to<JsonObject>();

  dataStore.lock();
  writeSource(root["source"].to<JsonObject>(), dataStore.trainStatus);
  const TrainAlerts* alerts = dataStore.trainAlerts.get();
  if (alerts == nullptr) {
    root["status"] = nullptr;
  } else {
    root["status"] = alerts->isDisrupted() ? "disrupted" : "normal";
    JsonArray segments = root["segments"].to<JsonArray>();
    for (const auto& seg : alerts->segments) {
      JsonObject s = segments.add<JsonObject>();
      s["line"] = seg.line;
      s["lineName"] = trainLineName(seg.line);
      s["direction"] = seg.direction;
      s["stations"] = seg.stations;
      s["freeBus"] = seg.freeBus;
      s["freeShuttle"] = seg.freeShuttle;
    }
    JsonArray messages = root["messages"].to<JsonArray>();
    for (const auto& msg : alerts->messages) messages.add(msg);
  }
  dataStore.unlock();

  response->setLength();
  request->send(response);
}

void StatusApi::refresh(AsyncWebServerRequest* request) {
  dataStore.requestRefresh();
  request->send(202, MimeType::JSON, "{\"success\":true}");
}

static void restartTask(void*) {
  vTaskDelay(pdMS_TO_TICKS(1500));  // Let the HTTP response go out first
  ESP.restart();
}

void StatusApi::restart(AsyncWebServerRequest* request) {
  request->send(202, MimeType::JSON, "{\"success\":true}");
  xTaskCreate(restartTask, "restart", 2048, nullptr, 1, nullptr);
}
