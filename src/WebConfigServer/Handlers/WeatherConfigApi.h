#pragma once

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include "AsyncJson.h"

class WeatherConfigApi {
 public:
  static void getWeatherRegion(AsyncWebServerRequest* request);
  static void saveWeatherRegion(AsyncWebServerRequest* request,
                                JsonVariant& json);

  static void getWeatherTown(AsyncWebServerRequest* request);
  static void saveWeatherTown(AsyncWebServerRequest* request,
                              JsonVariant& json);

  static void listWeatherRegions(AsyncWebServerRequest* request);
  static void listWeatherTowns(AsyncWebServerRequest* request);
};
