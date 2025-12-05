#include "LtaDatamallConfigApi.h"

#include "../../Config/ConfigManager.h"
#include "../HttpConstants.h"

void LtaDatamallConfigApi::saveApiKey(AsyncWebServerRequest* request,
                                      JsonVariant& jsonBody) {
  JsonObject reqBody = jsonBody.as<JsonObject>();
  if (reqBody["apiKey"].isUnbound()) {
    request->send(400, MimeType::JSON, "{ \"success\": false }");
    return;
  }

  configManager.saveLtaDatamallApiKey(reqBody["apiKey"]);
  request->send(200, MimeType::JSON, "{ \"success\": true }");
}

void LtaDatamallConfigApi::getSavedApiKey(AsyncWebServerRequest* request) {
  String apiKey = configManager.getLtaDatamallApiKey();
  if (apiKey.length() > 0) {
    request->send(200, MimeType::JSON, "{ \"isApiKeySaved\": true }");
  } else {
    request->send(200, MimeType::JSON, "{ \"isApiKeySaved\": false }");
  }
}

void LtaDatamallConfigApi::saveBusStopCode(AsyncWebServerRequest* request,
                                           JsonVariant& jsonBody) {
  JsonObject reqBody = jsonBody.as<JsonObject>();
  if (reqBody["busStopCode"].isUnbound()) {
    request->send(400, MimeType::JSON, "{ \"success\": false }");
    return;
  }

  configManager.saveBusStopCode(reqBody["busStopCode"]);
  request->send(200, MimeType::JSON, "{ \"success\": true }");
}

void LtaDatamallConfigApi::getSavedBusStopCode(AsyncWebServerRequest* request) {
  String busStopCode = configManager.getBusStopCode();
  if (busStopCode.length() > 0) {
    request->send(200, MimeType::JSON,
                  "{ \"busStopCode\": \"" + busStopCode + "\" }");
  } else {
    request->send(200, MimeType::JSON, "{ \"isApiKeySaved\": null }");
  }
}
