#pragma once

#include "../WebConfigServer.h"

class DisplayConfigApi {
 public:
  // GET  /api/config/display
  static void getDisplay(AsyncWebServerRequest* request);
  // POST /api/config/display (any subset of the GET fields)
  static void saveDisplay(AsyncWebServerRequest* request, JsonVariant& body);
};
