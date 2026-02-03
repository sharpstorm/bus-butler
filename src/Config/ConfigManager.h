#pragma once

#include "Preferences.h"

#define CONFIG_NAMESPACE "butler-config"

#define CONFIG_BUS_API_KEY "bus-api-key"
#define CONFIG_BRIGHTNESS "brightness"
#define CONFIG_BUS_STOP_CODE "busstop-code"
#define CONFIG_WEATHER_REGION "weather-region"
#define CONFIG_WEATHER_TOWN "weather-town"

class ConfigManager {
 private:
  Preferences preferences;

 public:
  ConfigManager();

  void saveLtaDatamallApiKey(String apiKey);
  String getLtaDatamallApiKey();

  void saveBrightness(uint8_t brightness);
  uint8_t getBrightness();

  void saveBusStopCode(String busStopCode);
  String getBusStopCode();

  void saveWeatherRegion(String region);
  String getWeatherRegion();

  void saveWeatherTown(String town);
  String getWeatherTown();
};

extern ConfigManager configManager;
