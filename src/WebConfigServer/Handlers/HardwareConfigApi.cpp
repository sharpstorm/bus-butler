#include "HardwareConfigApi.h"

#include "../../Config/ConfigManager.h"
#include "../../Screen/Screen.h"
#include "../HttpConstants.h"

void HardwareConfigApi::setScreenBrightness(AsyncWebServerRequest* request,
                                            JsonVariant& jsonBody) {
  JsonObject reqBody = jsonBody.as<JsonObject>();
  if (reqBody["brightness"].isUnbound()) {
    request->send(400, MimeType::JSON, "{ \"error\": \"brightness not set\" }");
    return;
  }

  uint8_t brightness = reqBody["brightness"].as<uint8_t>();
  configManager.saveBrightness(brightness);
  screen.setBrightness(brightness);
  request->send(200, MimeType::JSON,
                "{ \"newBrightness\": " + String(brightness) + " }");
}

void HardwareConfigApi::getScreenBrightness(AsyncWebServerRequest* request) {
  uint8_t brightness = screen.getBrightness();
  request->send(200, MimeType::JSON,
                "{ \"brightness\": " + String(brightness) + " }");
}
