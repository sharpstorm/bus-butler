#pragma once

#include <Arduino.h>

#include <vector>

using namespace std;

#define MORNING_STRING "6 am to midday"
#define AFTERNOON_STRING "midday to 6 pm"
#define NIGHT_STRING "6 pm to midnight"
#define MIDNIGHT_STRING "midnight to 6 am"

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
  float latitude = 0;
  float longitude = 0;
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
  WeatherCondition regionWest;
  WeatherCondition regionEast;
  WeatherCondition regionCentral;
  WeatherCondition regionSouth;
  WeatherCondition regionNorth;
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
  PeriodForecast midnight;
  // Same periods in the order NEA returns them (upcoming first)
  vector<PeriodForecast> periods;
};

// One day of the 4-day outlook
struct DayOutlook {
  String day;  // "Saturday"
  WeatherCondition condition;
  String summary;  // "Partly cloudy and warm"
  ValueRange temperature;
  ValueRange humidity;
  String windDirection;
  ValueRange windSpeed;
};

struct FourDayOutlookResponse {
  String updateTimestamp;
  vector<DayOutlook> days;
};

// Weather station metadata + latest reading (air temperature, humidity,
// rainfall all share this shape on data.gov.sg)
struct StationReading {
  String id;
  String name;
  float latitude = 0;
  float longitude = 0;
  float value = 0;
  bool hasValue = false;
};

struct StationReadingsResponse {
  String timestamp;
  String unit;
  vector<StationReading> stations;

  // Station closest to (lat, lon) that has a reading, or nullptr
  const StationReading* nearest(float lat, float lon) const;
};

// Latest 24-hour PSI per region
struct PsiResponse {
  String timestamp;
  int west = -1, east = -1, central = -1, north = -1, south = -1;
  int forRegion(const String& region) const;
};

// Latest hourly UV index
struct UvResponse {
  String hour;
  int value = -1;
};

// Condition for one of "west", "east", "central", "north", "south"
WeatherCondition periodConditionForRegion(const PeriodForecast& period,
                                          const String& region);
// "6 am to midday" -> "Morning", etc.
String periodShortLabel(const String& timePeriod);

// Utility functions
WeatherCondition parseWeatherCondition(const String& condition);
String weatherConditionToString(WeatherCondition condition);
String weatherConditionToShortString(WeatherCondition condition);
bool isRainyCondition(WeatherCondition condition);
bool isHeavyRainCondition(WeatherCondition condition);
bool isNightCondition(WeatherCondition condition);