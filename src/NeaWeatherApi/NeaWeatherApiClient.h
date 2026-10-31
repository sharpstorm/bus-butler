#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <memory>

#include "ArduinoJson.h"
#include "NeaWeatherApiTypes.h"

#define NEA_API_HOST "api-open.data.gov.sg"
#define TWO_HOUR_FORECAST_PATH "/v2/real-time/api/two-hr-forecast"
#define TWENTY_FOUR_HOUR_FORECAST_PATH \
  "/v2/real-time/api/twenty-four-hr-forecast"
#define FOUR_DAY_OUTLOOK_PATH "/v2/real-time/api/four-day-outlook"
#define AIR_TEMPERATURE_PATH "/v2/real-time/api/air-temperature"
#define RELATIVE_HUMIDITY_PATH "/v2/real-time/api/relative-humidity"
#define RAINFALL_PATH "/v2/real-time/api/rainfall"
#define PSI_PATH "/v2/real-time/api/psi"
#define UV_PATH "/v2/real-time/api/uv"
#define HTTPS_PORT 443
#define NEA_API_MAX_RETRIES 3
#define NEA_API_RETRY_DELAY_MS 1000

using namespace std;

class NeaWeatherClient {
 public:
  NeaWeatherClient();

  unique_ptr<TwoHourForecastResponse> getTwoHourForecast();
  unique_ptr<TwentyFourHourForecastResponse> getTwentyFourHourForecast();
  unique_ptr<FourDayOutlookResponse> getFourDayOutlook();
  // path: AIR_TEMPERATURE_PATH, RELATIVE_HUMIDITY_PATH or RAINFALL_PATH
  unique_ptr<StationReadingsResponse> getStationReadings(const char* path);
  unique_ptr<PsiResponse> getPsi();
  unique_ptr<UvResponse> getUv();

  // Get forecast for a specific area from two-hour forecast
  WeatherCondition getForecastForArea(const TwoHourForecastResponse* response,
                                      const String& area);

 private:
  WiFiClientSecure wifiClient;
  HTTPClient httpClient;

  static constexpr int MAX_RETRIES = 3;
  static constexpr int RETRY_DELAY_MS = 1000;

  unique_ptr<JsonDocument> performRequest(const char* path);
  PeriodForecast parsePeriodForecast(JsonObject& periodObj);
};

extern const char* dataGovCaCert;
extern NeaWeatherClient neaWeatherClient;
