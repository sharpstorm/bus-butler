#pragma once

#include <Arduino.h>

#include <vector>

using namespace std;

// Forecast conditions returned by NEA
enum class WeatherCondition {
  Fair,
  FairDay,
  FairNight,
  FairWarm,
  PartlyCloudy,
  PartlyCloudyDay,
  PartlyCloudyNight,
  Cloudy,
  Hazy,
  SlightlyHazy,
  Windy,
  Mist,
  Fog,
  LightRain,
  ModerateRain,
  HeavyRain,
  PassingShowers,
  LightShowers,
  Showers,
  HeavyShowers,
  Thunderstorm,
  HeavyThunderstorm,
  HeavyThunderstormWithGustyWinds,
  Unknown
};

// Two-hour forecast for a specific area
struct AreaForecast {
  String area;
  WeatherCondition condition;
};

// Two-hour forecast response
struct TwoHourForecastResponse {
  String validFrom;
  String validTo;
  String updateTimestamp;
  vector<AreaForecast> forecasts;
};

// Period forecast within 24-hour forecast
struct PeriodForecast {
  String timePeriod;  // "morning", "afternoon", "evening", "night"
  String regionWest;
  String regionEast;
  String regionCentral;
  String regionSouth;
  String regionNorth;
};

// Temperature/humidity range
struct ValueRange {
  int low;
  int high;
};

// 24-hour forecast response
struct TwentyFourHourForecastResponse {
  String validFrom;
  String validTo;
  String updateTimestamp;
  String generalForecast;
  ValueRange temperature;
  ValueRange humidity;
  PeriodForecast morning;
  PeriodForecast afternoon;
  PeriodForecast night;
};

// Utility functions
WeatherCondition parseWeatherCondition(const String& condition);
String weatherConditionToString(WeatherCondition condition);
String weatherConditionToShortString(WeatherCondition condition);