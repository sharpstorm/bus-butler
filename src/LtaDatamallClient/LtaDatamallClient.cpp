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

  int httpCode = -1;
  for (int attempt = 1; attempt <= LTA_DATAMALL_MAX_RETRIES; attempt++) {
    if (!httpClient.begin(wifiClient, LTA_DATAMALL_API_HOST, HTTPS_PORT, url,
                          true)) {
      lastError = -1;
      if (attempt < LTA_DATAMALL_MAX_RETRIES) {
        delay(LTA_DATAMALL_RETRY_DELAY_MS);
        continue;
      }
      return nullptr;
    }

    httpClient.addHeader("AccountKey", apiKey);
    httpClient.addHeader("Accept", "application/json");
    httpClient.setTimeout(10000);        // 10 second timeout
    httpClient.setConnectTimeout(3000);  // 3 second connection timeout

    httpCode = httpClient.GET();
    lastError = httpCode;
    if (httpCode == 200) {
      break;
    }

    // -1 here is almost always a failed TLS handshake (CA mismatch) or DNS.
    Serial.printf("DataMall BusArrival %s failed: %d (%s)\n", url, httpCode,
                  HTTPClient::errorToString(httpCode).c_str());

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
  httpClient.end();  // Body is fully parsed; free the TLS session now
  if (error) {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
    lastError = -2;
    return nullptr;
  }

  if (!doc.is<JsonObject>()) {
    lastError = -2;
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

  return resp;
}

int LtaDatamallClient::fetchJson(const String& pathAndQuery, JsonDocument& doc,
                                 JsonDocument* filter) {
  if (apiKey.length() == 0) {
    return -1;
  }

  int httpCode = -1;
  for (int attempt = 1; attempt <= LTA_DATAMALL_MAX_RETRIES; attempt++) {
    if (!httpClient.begin(wifiClient, LTA_DATAMALL_API_HOST, HTTPS_PORT,
                          pathAndQuery, true)) {
      httpCode = -1;
      lastError = -1;
      delay(LTA_DATAMALL_RETRY_DELAY_MS);
      continue;
    }

    httpClient.addHeader("AccountKey", apiKey);
    httpClient.addHeader("Accept", "application/json");
    httpClient.setTimeout(15000);
    httpClient.setConnectTimeout(5000);

    httpCode = httpClient.GET();
    lastError = httpCode;
    if (httpCode == 200) {
      DeserializationError error =
          filter == nullptr
              ? deserializeJson(doc, httpClient.getStream())
              : deserializeJson(doc, httpClient.getStream(),
                                DeserializationOption::Filter(*filter));
      httpClient.end();
      if (error) {
        Serial.printf("DataMall JSON error on %s: %s\n", pathAndQuery.c_str(),
                      error.c_str());
        lastError = -2;
        return -2;
      }
      return 200;
    }

    Serial.printf("DataMall %s failed: %d (%s)\n", pathAndQuery.c_str(),
                  httpCode, HTTPClient::errorToString(httpCode).c_str());
    httpClient.end();
    if (httpCode >= 400 && httpCode < 500) {
      return httpCode;  // Bad key or bad request: retrying won't help
    }
    if (attempt < LTA_DATAMALL_MAX_RETRIES) {
      delay(LTA_DATAMALL_RETRY_DELAY_MS);
    }
  }
  return httpCode;
}

unique_ptr<TrainAlerts> LtaDatamallClient::getTrainServiceAlerts() {
  JsonDocument doc;
  if (fetchJson(LTA_DATAMALL_TRAIN_ALERTS_API_PATH, doc) != 200) {
    return nullptr;
  }

  // "value" is an object in the current API; older docs show it wrapped in
  // an array, so accept both.
  JsonObject value = doc["value"].is<JsonArray>()
                         ? doc["value"][0].as<JsonObject>()
                         : doc["value"].as<JsonObject>();
  if (value.isNull()) {
    return nullptr;
  }

  unique_ptr<TrainAlerts> alerts = make_unique<TrainAlerts>();
  // Documented as a number; accept a numeric string too.
  JsonVariant status = value["Status"];
  alerts->status = status.is<const char*>() ? atoi(status.as<const char*>())
                                            : (status | 1);

  for (JsonObject seg : value["AffectedSegments"].as<JsonArray>()) {
    TrainAlertSegment s;
    s.line = seg["Line"] | "";
    s.direction = seg["Direction"] | "";
    s.stations = seg["Stations"] | "";
    s.freeBus = seg["FreePublicBus"] | "";
    s.freeShuttle = seg["FreeMRTShuttle"] | "";
    if (s.line.length() > 0) {
      alerts->segments.push_back(s);
    }
  }

  for (JsonObject msg : value["Message"].as<JsonArray>()) {
    String content = msg["Content"] | "";
    content.trim();
    if (content.length() > 0) {
      alerts->messages.push_back(content);
    }
  }

  return alerts;
}

int LtaDatamallClient::lookupBusStopName(const String& busStopCode,
                                         String& outName) {
  JsonDocument filter;
  filter["value"][0]["BusStopCode"] = true;
  filter["value"][0]["Description"] = true;
  filter["value"][0]["RoadName"] = true;

  for (int page = 0; page < LTA_DATAMALL_BUS_STOPS_MAX_PAGES; page++) {
    JsonDocument doc;
    String path = String(LTA_DATAMALL_BUS_STOPS_API_PATH) + "?$skip=" +
                  String(page * LTA_DATAMALL_BUS_STOPS_PAGE_SIZE);
    if (fetchJson(path, doc, &filter) != 200) {
      return -1;
    }

    JsonArray stops = doc["value"].as<JsonArray>();
    if (stops.size() == 0) {
      return 0;  // Ran off the end of the dataset
    }

    for (JsonObject stop : stops) {
      if (busStopCode == (stop["BusStopCode"] | "")) {
        outName = stop["Description"] | "";
        if (outName.length() == 0) {
          outName = stop["RoadName"] | "";
        }
        return 1;
      }
    }
  }
  return 0;
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
