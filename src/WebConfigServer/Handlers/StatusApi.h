#pragma once

#include "../WebConfigServer.h"

class StatusApi {
 public:
  // GET /api/status: device health and data source status
  static void getStatus(AsyncWebServerRequest* request);
  // GET /api/mrt: latest MRT service alerts the device has
  static void getMrt(AsyncWebServerRequest* request);
  // POST /api/refresh: refetch everything now
  static void refresh(AsyncWebServerRequest* request);
  // POST /api/restart: reboot after the response is sent (e.g. new WiFi)
  static void restart(AsyncWebServerRequest* request);
};
