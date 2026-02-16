#include "NeaWeatherApiClient.h"

#include <ArduinoJson.h>

#include "../Util/MemoryPolyfill.h"
#include "../Wifi/WifiManager.h"

NeaWeatherClient::NeaWeatherClient() {
  // Use insecure mode - data.gov.sg uses Let's Encrypt which rotates frequently
  wifiClient.setCACert(dataGovCaCert);
  httpClient.useHTTP10(true);
}

unique_ptr<JsonDocument> NeaWeatherClient::performRequest(const char* path) {
  String responseBody;
  int httpCode = -1;

  for (int attempt = 1; attempt <= NEA_API_MAX_RETRIES; attempt++) {
    if (!httpClient.begin(wifiClient, NEA_API_HOST, 443, path, true)) {
      Serial.println("httpClient.begin() failed");
      if (attempt < NEA_API_MAX_RETRIES) {
        delay(NEA_API_RETRY_DELAY_MS);
        continue;
      }
      return nullptr;
    }

    httpClient.addHeader("Host", NEA_API_HOST);
    httpClient.addHeader("Connection", "close");
    httpClient.addHeader("User-Agent", "ESP32-BusButler/1.0");
    httpClient.setTimeout(10000);
    httpClient.setConnectTimeout(3000);

    httpCode = httpClient.GET();

    if (httpCode != 200) {
      httpClient.end();

      // Errors that will not resolve, no point in retrying
      if (httpCode >= 400 && httpCode < 500) {
        return nullptr;
      }

      if (attempt < MAX_RETRIES) {
        delay(RETRY_DELAY_MS);
        continue;
      }
      return nullptr;
    }

    unique_ptr<JsonDocument> doc = make_unique<JsonDocument>();
    DeserializationError error = deserializeJson(*doc, httpClient.getStream());
    httpClient.end();

    if (error) {
      Serial.print("JSON deserialization failed: ");
      Serial.println(error.c_str());
      continue;
    }

    return doc;
  }

  return nullptr;
}

unique_ptr<TwoHourForecastResponse> NeaWeatherClient::getTwoHourForecast() {
  unique_ptr<JsonDocument> doc = performRequest(TWO_HOUR_FORECAST_PATH);
  if (doc == nullptr) {
    return nullptr;
  }

  if (!doc->is<JsonObject>()) {
    Serial.println("JSON root is not an object");
    return nullptr;
  }

  JsonObject root = doc->as<JsonObject>();

  // Check for API error
  if (root["code"].is<int>() && root["code"].as<int>() != 0) {
    Serial.printf("API error: %s\n", root["message"].as<const char*>());
    return nullptr;
  }

  // Navigate to data
  if (!root["data"].is<JsonObject>()) {
    Serial.println("Missing 'data' field");
    return nullptr;
  }

  JsonObject data = root["data"].as<JsonObject>();
  if (!data["items"].is<JsonArray>()) {
    Serial.println("Missing 'items' array");
    return nullptr;
  }

  JsonArray items = data["items"].as<JsonArray>();
  if (items.size() == 0) {
    Serial.println("Empty items array");
    return nullptr;
  }

  // Get the first (most recent) record
  JsonObject record = items[0].as<JsonObject>();

  unique_ptr<TwoHourForecastResponse> resp =
      make_unique<TwoHourForecastResponse>();
  if (resp == nullptr) {
    return nullptr;
  }

  // Parse validity period
  if (record["valid_period"].is<JsonObject>()) {
    JsonObject period = record["valid_period"].as<JsonObject>();
    resp->validFrom = period["start"].as<String>();
    resp->validTo = period["end"].as<String>();
  }
  resp->updateTimestamp = record["update_timestamp"].as<String>();

  // Parse forecasts
  if (record["forecasts"].is<JsonArray>()) {
    JsonArray forecasts = record["forecasts"].as<JsonArray>();
    for (JsonObject forecast : forecasts) {
      AreaForecast area;
      area.area = forecast["area"].as<String>();
      area.condition = parseWeatherCondition(forecast["forecast"].as<String>());
      resp->forecasts.push_back(area);
    }
  }

  Serial.printf("Parsed %d area forecasts\n", resp->forecasts.size());
  return resp;
}

unique_ptr<TwentyFourHourForecastResponse>
NeaWeatherClient::getTwentyFourHourForecast() {
  unique_ptr<JsonDocument> doc = performRequest(TWENTY_FOUR_HOUR_FORECAST_PATH);
  if (doc == nullptr) {
    return nullptr;
  }

  if (!doc->is<JsonObject>()) {
    return nullptr;
  }

  JsonObject root = doc->as<JsonObject>();

  if (root["code"].is<int>() && root["code"].as<int>() != 0) {
    Serial.printf("API error: %s\n", root["message"].as<const char*>());
    return nullptr;
  }

  if (!root["data"].as<JsonObject>()) {
    return nullptr;
  }

  JsonObject data = root["data"].as<JsonObject>();
  if (!data["records"].is<JsonArray>()) {
    return nullptr;
  }

  JsonArray records = data["records"].as<JsonArray>();
  if (records.size() == 0) {
    return nullptr;
  }

  JsonObject record = records[0].as<JsonObject>();

  unique_ptr<TwentyFourHourForecastResponse> resp =
      make_unique<TwentyFourHourForecastResponse>();
  if (resp == nullptr) {
    return nullptr;
  }

  // Parse validity period
  if (record["validPeriod"].is<JsonObject>()) {
    JsonObject period = record["validPeriod"].as<JsonObject>();
    resp->validFrom = period["start"].as<String>();
    resp->validTo = period["end"].as<String>();
  }
  resp->updateTimestamp = record["updatedTimestamp"].as<String>();

  // Parse general forecast
  if (record["general"].is<JsonObject>()) {
    JsonObject general = record["general"].as<JsonObject>();

    resp->generalForecast = general["forecast"]["text"].as<String>();

    // Temperature
    if (general["temperature"].is<JsonObject>()) {
      JsonObject temp = general["temperature"].as<JsonObject>();
      resp->temperature.low = temp["low"].as<int>();
      resp->temperature.high = temp["high"].as<int>();
    }

    // Humidity
    if (general["relativeHumidity"].is<JsonObject>()) {
      JsonObject humid = general["relativeHumidity"].as<JsonObject>();
      resp->humidity.low = humid["low"].as<int>();
      resp->humidity.high = humid["high"].as<int>();
    }
  }

  // Parse period forecasts
  if (record["periods"].is<JsonArray>()) {
    JsonArray periods = record["periods"].as<JsonArray>();

    for (JsonObject periodObj : periods) {
      String timePeriod = periodObj["timePeriod"]["text"].as<String>();
      timePeriod.toLowerCase();

      PeriodForecast pf = parsePeriodForecast(periodObj);
      pf.timePeriod = timePeriod;

      if (timePeriod.indexOf(MIDNIGHT_STRING) >= 0) {
        resp->midnight = pf;
      } else if (timePeriod.indexOf(MORNING_STRING) >= 0) {
        resp->morning = pf;
      } else if (timePeriod.indexOf(AFTERNOON_STRING) >= 0) {
        resp->afternoon = pf;
      } else if (timePeriod.indexOf(NIGHT_STRING) >= 0) {
        resp->night = pf;
      }
    }
  }

  Serial.println("Parsed 24-hour forecast");
  return resp;
}

PeriodForecast NeaWeatherClient::parsePeriodForecast(JsonObject& periodObj) {
  PeriodForecast pf;

  if (periodObj["regions"].is<JsonObject>()) {
    JsonObject regions = periodObj["regions"].as<JsonObject>();
    pf.regionWest = parseWeatherCondition(regions["west"]["text"].as<String>());
    pf.regionEast = parseWeatherCondition(regions["east"]["text"].as<String>());
    pf.regionCentral =
        parseWeatherCondition(regions["central"]["text"].as<String>());
    pf.regionSouth =
        parseWeatherCondition(regions["south"]["text"].as<String>());
    pf.regionNorth =
        parseWeatherCondition(regions["north"]["text"].as<String>());
  }

  return pf;
}

WeatherCondition NeaWeatherClient::getForecastForArea(
    const TwoHourForecastResponse* response, const String& area) {
  if (response == nullptr) {
    return WeatherCondition::Unknown;
  }

  String searchArea = area;
  searchArea.toLowerCase();

  for (const auto& forecast : response->forecasts) {
    String forecastArea = forecast.area;
    forecastArea.toLowerCase();

    if (forecastArea == searchArea || forecastArea.indexOf(searchArea) >= 0) {
      return forecast.condition;
    }
  }

  return WeatherCondition::Unknown;
}