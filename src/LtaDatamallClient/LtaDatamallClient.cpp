#include "LtaDatamallClient.h"

#include <ArduinoJson.h>

#include "../Config/ConfigManager.h"
#include "../Util/MemoryPolyfill.h"
#include "../Util/Time.h"

LtaDatamallClient::LtaDatamallClient() : apiKey("") {
  // We cache the API key into this class
  reconfigure();
  httpClient.useHTTP10(true);
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
           busStopCode.c_str());

  for (int attempt = 1; attempt <= LTA_DATAMALL_MAX_RETRIES; attempt++) {
    if (!httpClient.begin(wifiClient, LTA_DATAMALL_API_HOST, HTTPS_PORT, url,
                          true)) {
      if (attempt < LTA_DATAMALL_MAX_RETRIES) {
        delay(LTA_DATAMALL_RETRY_DELAY_MS);
        continue;
      }
      return nullptr;
    }

    httpClient.addHeader("Host", LTA_DATAMALL_API_HOST);
    httpClient.addHeader("Connection", "close");
    httpClient.addHeader("AccountKey", apiKey);
    httpClient.addHeader("User-Agent", "axios/3.0.5");
    httpClient.setTimeout(10000);        // 10 second timeout
    httpClient.setConnectTimeout(3000);  // 3 second connection timeout

    int httpCode = httpClient.GET();
    if (httpCode == 200) {
      break;
    }

    // Not successful, disconnect and figure out if we should retry
    httpClient.end();
    if (httpCode >= 400 && httpCode < 500) {
      return nullptr;
    }

    if (attempt < LTA_DATAMALL_MAX_RETRIES) {
      delay(LTA_DATAMALL_RETRY_DELAY_MS);
    } else {
      return nullptr;
    }
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, httpClient.getStream());
  if (error) {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
    return nullptr;
  }

  if (!doc.is<JsonObject>()) {
    return nullptr;
  }

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

  httpClient.end();
  return resp;
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

LtaDatamallClient ltaDatamallClient;
