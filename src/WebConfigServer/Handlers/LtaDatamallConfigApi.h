#pragma once

#include "ArduinoJson.h"

// Server
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"

class LtaDatamallConfigApi {
 public:
  static void saveApiKey(AsyncWebServerRequest* request, JsonVariant& json);
  static void getSavedApiKey(AsyncWebServerRequest* request);
};
