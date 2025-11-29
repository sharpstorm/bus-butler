#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <memory>

#include "ArduinoJson.h"
#include "BusArrivalApiTypes.h"

#define LTA_DATAMALL_API_HOST "datamall2.mytransport.sg"
#define LTA_DATAMALL_BUS_ARRIVAL_API_PATH "/ltaodataservice/v3/BusArrival"
#define HTTPS_PORT 443

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
};

extern const char* ltaDatamallCaCert;
