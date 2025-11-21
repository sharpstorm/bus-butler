#pragma once

#include "ESPAsyncWebServer.h"

class StaticServeHandler {
 public:
  static void handleHTML(AsyncWebServerRequest* request);
  static void handleJS(AsyncWebServerRequest* request);
};
