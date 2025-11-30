#pragma once

#include "ArduinoJson.h"

// Server
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"

class HardwareConfigApi {
 public:
  static void setScreenBrightness(AsyncWebServerRequest* request,
                                  JsonVariant& json);
  static void getScreenBrightness(AsyncWebServerRequest* request);
};
