#pragma once

#include "ArduinoJson.h"

// Server
#include <ESPAsyncWebServer.h>

#include "../../LtaDatamallClient/LtaDatamallClient.h"
#include "../../NeaWeatherApi/NeaWeatherApiClient.h"

class ConfigTestApi {
 public:
  static void queryBusArrival(AsyncWebServerRequest* request,
                              JsonVariant& json);
  static void queryWeather(AsyncWebServerRequest* request);

 private:
  static LtaDatamallClient ltaDatamallClient;
  static bool initDatamallClient();

  // Bus Arrival API
  static volatile bool queryBusArrivalCompleted;
  static void queryBusArrivalWorker(void* pvParameters);
  static void formatBusResponse(const JsonObject& output,
                                const BusArrivalDetail& arrivalDetails,
                                long timeNow);

  // Weather API
  static NeaWeatherClient weatherClient;
  static volatile bool queryWeatherCompleted;
  static void queryWeatherWorker(void* pvParameters);
};
