#pragma once

#include "ArduinoJson.h"

// Server
#include <ESPAsyncWebServer.h>

#include "../../LtaDatamallClient/LtaDatamallClient.h"

class ConfigTestApi {
 public:
  static void queryBusArrival(AsyncWebServerRequest* request,
                              JsonVariant& json);

 private:
  static LtaDatamallClient ltaDatamallClient;
  static bool initDatamallClient();

  // Bus Arrival API
  static volatile bool queryBusArrivalCompleted;
  static void queryBusArrivalWorker(void* pvParameters);
  static void formatBusResponse(const JsonObject& output,
                                const BusArrivalDetail& arrivalDetails,
                                long timeNow);
};
