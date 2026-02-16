#include "BusServiceConfigApi.h"

#include "../../Config/ConfigManager.h"
#include "../HttpConstants.h"
#include "ArduinoJson.h"
#include "AsyncJson.h"

void BusServiceConfigApi::getBusServiceOrder(AsyncWebServerRequest* request) {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject root = response->getRoot().as<JsonObject>();

  BusServiceOrderConfig order = configManager.getBusServiceOrder();
  if (order.services == nullptr) {
    root["order"].to<JsonArray>().clear();
  } else {
    JsonArray arr = root["order"].to<JsonArray>();
    for (int i = 0; i < order.size; i++) {
      arr.add(order.services[i]);
    }
  }

  response->setLength();
  request->send(response);
}

void BusServiceConfigApi::saveBusServiceOrder(AsyncWebServerRequest* request,
                                              uint8_t* data, size_t len,
                                              size_t index, size_t total) {
  if (index + len != total) {
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data, len);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    request->send(400, MimeType::JSON, "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (doc["order"].isUnbound()) {
    request->send(400, MimeType::JSON, "{\"error\":\"Missing order field\"}");
    return;
  }

  if (!doc["order"].is<JsonArray>()) {
    request->send(400, MimeType::JSON,
                  "{\"error\":\"Order must be JSON array\"}");
    return;
  }

  JsonArray order = doc["order"].as<JsonArray>();

  // Save as command separated
  int orderCount = 0;
  for (JsonVariant value : order) {
    orderCount++;
  }

  String* services = new String[orderCount];
  int i = 0;
  for (JsonVariant value : order) {
    services[i] = value.as<String>();
    i++;
  }
  configManager.saveBusServiceOrder(services, orderCount);

  AsyncJsonResponse* response = new AsyncJsonResponse();
  JsonObject root = response->getRoot().as<JsonObject>();
  root["success"] = true;
  response->setLength();
  request->send(response);
}
