#include "LtaDatamallClient.h"

#include <ArduinoJson.h>

#include "../Config/ConfigManager.h"
#include "../Util/MemoryPolyfill.h"
#include "../Util/Time.h"

LtaDatamallClient::LtaDatamallClient() : apiKey("") {
  // We cache the API key into this class
  reconfigure();
}

bool LtaDatamallClient::isConfigured() { return apiKey.length() > 0; }

void LtaDatamallClient::reconfigure() {
  apiKey = configManager.getLtaDatamallApiKey();
  wifiClient.setCACert(ltaDatamallCaCert);
}

unique_ptr<BusArrivalApiResponse> LtaDatamallClient::getBusArrival(
    String busStopCode) {
  if (apiKey.length() == 0) {
    return nullptr;
  }

  char url[60];
  snprintf(url, 60, "%s?BusStopCode=%s", LTA_DATAMALL_BUS_ARRIVAL_API_PATH,
           busStopCode);
  Serial.println(url);
  if (!httpClient.begin(wifiClient, LTA_DATAMALL_API_HOST, HTTPS_PORT, url,
                        true)) {
    return nullptr;
  }

  httpClient.addHeader("Host", LTA_DATAMALL_API_HOST);
  httpClient.addHeader("Connection", "close");
  httpClient.addHeader("AccountKey", apiKey);
  httpClient.addHeader("User-Agent", "axios/3.0.5");

  int httpCode = httpClient.GET();
  httpClient.end();

  if (httpCode != 200) {
    return nullptr;
  }

  JsonDocument doc;
  deserializeJson(doc, httpClient.getStream());
  JsonObject respRoot = doc.as<JsonObject>();

  unique_ptr<BusArrivalApiResponse> resp = make_unique<BusArrivalApiResponse>();
  BusArrivalApiResponse* respJson = resp.get();
  respJson->busStopCode = respRoot["BusStopCode"].as<String>();

  JsonArray services = respRoot["Services"].as<JsonArray>();
  for (JsonObject serviceObj : services) {
    String serviceNo = serviceObj["ServiceNo"].as<String>();
    String company = serviceObj["Operator"].as<String>();

    JsonObject nextBus1 = serviceObj["NextBus"].as<JsonObject>();
    JsonObject nextBus2 = serviceObj["NextBus2"].as<JsonObject>();
    JsonObject nextBus3 = serviceObj["NextBus3"].as<JsonObject>();

    respJson->services.push_back(
        {serviceNo, company, fromJsonResponse(nextBus1),
         fromJsonResponse(nextBus2), fromJsonResponse(nextBus3)});
  }

  return move(resp);
}

BusArrivalDetail LtaDatamallClient::fromJsonResponse(JsonObject resp) {
  if (resp.isNull()) {
    // Return timestamp 0 to signal no data
    return BusArrivalDetail{
        "",    "", BusLoad::LimitedStandingAvailable, BusType::SingleDeck,
        false, 0,
    };
  }

  return BusArrivalDetail{
      resp["OriginCode"].as<String>(),
      resp["DestinationCode"].as<String>(),
      toBusLoadEnum(resp["Load"].as<String>()),
      toBusTypeEnum(resp["Type"].as<String>()),
      resp["Feature"].as<String>() == "WAB",
      TimeUtil::iso8601StringToUnixTime(resp["EstimatedArrival"].as<String>()),
  };
}
