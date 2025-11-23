#pragma once

#include "ArduinoJson.h"

// Server
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"

class WifiConfigApi {
 public:
  static void scanWifiNetworks(AsyncWebServerRequest* request);
  static void saveWifiNetwork(AsyncWebServerRequest* request,
                              JsonVariant& json);
  static void getSavedWifiNetwork(AsyncWebServerRequest* request);
};
