#include "NeaWeatherApiClient.h"

#include <ArduinoJson.h>

#include "../Util/MemoryPolyfill.h"
#include "../Wifi/WifiManager.h"

NeaWeatherClient::NeaWeatherClient() {
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

  // Town coordinates, used to pick the nearest weather station
  for (JsonObject meta : data["area_metadata"].as<JsonArray>()) {
    String name = meta["name"] | "";
    for (auto& area : resp->forecasts) {
      if (area.area == name) {
        area.latitude = meta["label_location"]["latitude"] | 0.0f;
        area.longitude = meta["label_location"]["longitude"] | 0.0f;
        break;
      }
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
      resp->periods.push_back(pf);

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

unique_ptr<FourDayOutlookResponse> NeaWeatherClient::getFourDayOutlook() {
  unique_ptr<JsonDocument> doc = performRequest(FOUR_DAY_OUTLOOK_PATH);
  if (doc == nullptr) {
    return nullptr;
  }

  JsonObject record = (*doc)["data"]["records"][0].as<JsonObject>();
  if (record.isNull()) {
    return nullptr;
  }

  unique_ptr<FourDayOutlookResponse> resp = make_unique<FourDayOutlookResponse>();
  resp->updateTimestamp = record["updatedTimestamp"] | "";
  for (JsonObject f : record["forecasts"].as<JsonArray>()) {
    DayOutlook day;
    day.day = f["day"] | "";
    day.condition = parseWeatherCondition(f["forecast"]["text"] | "");
    day.summary = f["forecast"]["summary"] | "";
    day.temperature = {f["temperature"]["low"] | 0, f["temperature"]["high"] | 0};
    day.humidity = {f["relativeHumidity"]["low"] | 0,
                    f["relativeHumidity"]["high"] | 0};
    day.windDirection = f["wind"]["direction"] | "";
    day.windSpeed = {f["wind"]["speed"]["low"] | 0, f["wind"]["speed"]["high"] | 0};
    resp->days.push_back(day);
  }
  return resp;
}

unique_ptr<StationReadingsResponse> NeaWeatherClient::getStationReadings(
    const char* path) {
  unique_ptr<JsonDocument> doc = performRequest(path);
  if (doc == nullptr) {
    return nullptr;
  }

  JsonObject data = (*doc)["data"].as<JsonObject>();
  if (data.isNull()) {
    return nullptr;
  }

  unique_ptr<StationReadingsResponse> resp =
      make_unique<StationReadingsResponse>();
  resp->unit = data["readingUnit"] | "";
  for (JsonObject st : data["stations"].as<JsonArray>()) {
    StationReading reading;
    reading.id = st["id"] | "";
    reading.name = st["name"] | "";
    reading.latitude = st["location"]["latitude"] | 0.0f;
    reading.longitude = st["location"]["longitude"] | 0.0f;
    resp->stations.push_back(reading);
  }

  JsonObject latest = data["readings"][0].as<JsonObject>();
  resp->timestamp = latest["timestamp"] | "";
  for (JsonObject r : latest["data"].as<JsonArray>()) {
    String id = r["stationId"] | "";
    for (auto& station : resp->stations) {
      if (station.id == id) {
        station.value = r["value"] | 0.0f;
        station.hasValue = true;
        break;
      }
    }
  }
  return resp;
}

unique_ptr<PsiResponse> NeaWeatherClient::getPsi() {
  unique_ptr<JsonDocument> doc = performRequest(PSI_PATH);
  if (doc == nullptr) {
    return nullptr;
  }

  JsonObject item = (*doc)["data"]["items"][0].as<JsonObject>();
  JsonObject psi = item["readings"]["psi_twenty_four_hourly"].as<JsonObject>();
  if (psi.isNull()) {
    return nullptr;
  }

  unique_ptr<PsiResponse> resp = make_unique<PsiResponse>();
  resp->timestamp = item["timestamp"] | "";
  resp->west = psi["west"] | -1;
  resp->east = psi["east"] | -1;
  resp->central = psi["central"] | -1;
  resp->north = psi["north"] | -1;
  resp->south = psi["south"] | -1;
  return resp;
}

unique_ptr<UvResponse> NeaWeatherClient::getUv() {
  unique_ptr<JsonDocument> doc = performRequest(UV_PATH);
  if (doc == nullptr) {
    return nullptr;
  }

  // index[] is newest first
  JsonObject latest = (*doc)["data"]["records"][0]["index"][0].as<JsonObject>();
  if (latest.isNull()) {
    return nullptr;
  }

  unique_ptr<UvResponse> resp = make_unique<UvResponse>();
  resp->hour = latest["hour"] | "";
  resp->value = latest["value"] | -1;
  return resp;
}

NeaWeatherClient neaWeatherClient;
