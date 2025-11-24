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
