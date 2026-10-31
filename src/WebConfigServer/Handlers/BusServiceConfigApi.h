#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class BusServiceConfigApi {
 public:
  static void getBusServiceOrder(AsyncWebServerRequest* request);
  static void saveBusServiceOrder(AsyncWebServerRequest* request, uint8_t* data,
                                  size_t len, size_t index, size_t total);
};
