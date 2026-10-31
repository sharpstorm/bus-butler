#include "StopsConfigApi.h"

#include "../../Config/ConfigManager.h"
#include "../../Data/DataStore.h"
#include "../HttpConstants.h"

static bool isValidStopCode(const String& code) {
  if (code.length() != 5) return false;
  for (size_t i = 0; i < code.length(); i++) {
    if (!isDigit(code[i])) return false;
  }
  return true;
}

void StopsConfigApi::getStops(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject root = response->getRoot().to<JsonObject>();
  JsonArray stops = root["stops"].to<JsonArray>();
  for (const auto& stop : configManager.getBusStops()) {
    JsonObject item = stops.add<JsonObject>();
    item["code"] = stop.code;
    item["name"] = stop.name;
  }
  root["active"] = configManager.getActiveBusStopIndex();
  root["max"] = MAX_BUS_STOPS;
  response->setLength();
  request->send(response);
}

void StopsConfigApi::saveStops(AsyncWebServerRequest* request,
                               JsonVariant& body) {
  JsonArray input = body["stops"].as<JsonArray>();
  if (input.isNull()) {
    request->send(400, MimeType::JSON, "{\"error\":\"stops must be an array\"}");
    return;
  }

  vector<BusStopConfig> stops;
  for (JsonObject item : input) {
    String code = item["code"] | "";
    String name = item["name"] | "";
    code.trim();
    name.trim();
    if (!isValidStopCode(code)) {
      request->send(400, MimeType::JSON,
                    "{\"error\":\"Bus stop codes are 5 digits\"}");
      return;
    }
    bool duplicate = false;
    for (const auto& s : stops) duplicate |= s.code == code;
    if (duplicate) continue;
    if (name.length() > 40) name = name.substring(0, 40);
    stops.push_back({code, name});
    if (stops.size() >= MAX_BUS_STOPS) break;
  }

  configManager.saveBusStops(stops);
  if (!body["active"].isNull()) {
    uint8_t active = body["active"] | 0;
    configManager.saveActiveBusStopIndex(active < stops.size() ? active : 0);
  }
  request->send(200, MimeType::JSON, "{\"success\":true}");
}

void StopsConfigApi::startLookup(AsyncWebServerRequest* request,
                                 JsonVariant& body) {
  String code = body["code"] | "";
  code.trim();
  if (!isValidStopCode(code)) {
    request->send(400, MimeType::JSON,
                  "{\"error\":\"Bus stop codes are 5 digits\"}");
    return;
  }
  dataStore.requestStopLookup(code);
  request->send(202, MimeType::JSON, "{\"status\":\"pending\"}");
}

void StopsConfigApi::getLookup(AsyncWebServerRequest* request) {
  if (!request->hasParam("code")) {
    request->send(400, MimeType::JSON, "{\"error\":\"code is required\"}");
    return;
  }
  String code = request->getParam("code")->value();
  String name;
  StopLookupState state = dataStore.getStopLookup(code, name);

  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject root = response->getRoot().to<JsonObject>();
  switch (state) {
    case StopLookupState::Pending:
      root["status"] = "pending";
      break;
    case StopLookupState::Found:
      root["status"] = "found";
      root["name"] = name;
      break;
    case StopLookupState::NotFound:
      root["status"] = "not_found";
      break;
    case StopLookupState::Error:
      root["status"] = "error";
      break;
    default:
      root["status"] = "unknown";
      break;
  }
  response->setLength();
  request->send(response);
}
