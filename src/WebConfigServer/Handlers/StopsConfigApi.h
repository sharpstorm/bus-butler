#pragma once

#include "../WebConfigServer.h"

class StopsConfigApi {
 public:
  // GET  /api/config/stops  -> { stops: [{code, name}], active }
  static void getStops(AsyncWebServerRequest* request);
  // POST /api/config/stops  { stops: [{code, name}], active? }
  static void saveStops(AsyncWebServerRequest* request, JsonVariant& body);
  // POST /api/stops/lookup  { code } -> starts a name lookup on the device
  static void startLookup(AsyncWebServerRequest* request, JsonVariant& body);
  // GET  /api/stops/lookup?code=17179 -> { status, name }
  static void getLookup(AsyncWebServerRequest* request);
};
