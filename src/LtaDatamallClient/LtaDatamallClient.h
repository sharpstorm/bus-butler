#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <memory>

#include "ArduinoJson.h"
#include "BusArrivalApiTypes.h"
#include "TrainAlertTypes.h"

#define LTA_DATAMALL_API_HOST "datamall2.mytransport.sg"
#define LTA_DATAMALL_BUS_ARRIVAL_API_PATH "/ltaodataservice/v3/BusArrival"
#define LTA_DATAMALL_TRAIN_ALERTS_API_PATH "/ltaodataservice/TrainServiceAlerts"
#define LTA_DATAMALL_BUS_STOPS_API_PATH "/ltaodataservice/BusStops"
#define LTA_DATAMALL_BUS_STOPS_PAGE_SIZE 500
#define LTA_DATAMALL_BUS_STOPS_MAX_PAGES 16
#define HTTPS_PORT 443
#define LTA_DATAMALL_MAX_RETRIES 3
#define LTA_DATAMALL_RETRY_DELAY_MS 1000

using namespace std;

class LtaDatamallClient {
 private:
  String apiKey;
  WiFiClientSecure wifiClient;
  HTTPClient httpClient;

  BusArrivalDetail fromJsonResponse(JsonObject);

 public:
  LtaDatamallClient();
  bool isConfigured();
  void reconfigure();

  unique_ptr<BusArrivalApiResponse> getBusArrival(String busStopCode);

  // GET any DataMall path (with query) into doc. Returns the HTTP status, or
  // a negative value for connection / parse failures.
  int fetchJson(const String& pathAndQuery, JsonDocument& doc,
                JsonDocument* filter = nullptr);

  unique_ptr<TrainAlerts> getTrainServiceAlerts();

  // Pages through the BusStops dataset looking for the code.
  // Returns 1 if found (outName set), 0 if not found, -1 on error.
  int lookupBusStopName(const String& busStopCode, String& outName);
};

extern const char* ltaDatamallCaCert;
extern LtaDatamallClient ltaDatamallClient;
