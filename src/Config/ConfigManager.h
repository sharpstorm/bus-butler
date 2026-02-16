#pragma once

#include <memory>

#include "Preferences.h"

#define CONFIG_NAMESPACE "butler-config"

// System
#define CONFIG_BRIGHTNESS "brightness"

// Bus
#define CONFIG_BUS_API_KEY "bus-api-key"
#define CONFIG_BUS_STOP_CODE "busstop-code"
#define CONFIG_BUS_SERVICE_ORDER "bus-svc-order"

// Weathers
#define CONFIG_WEATHER_REGION "weather-region"
#define CONFIG_WEATHER_TOWN "weather-town"

using namespace std;

struct BusServiceOrderConfig {
  unique_ptr<String[]> services;
  int size;
};

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

  // Stored as comma separated, <length>,<svc 1>,<svc 2>,...
  void saveBusServiceOrder(String serviceOrder[], int size);
  BusServiceOrderConfig getBusServiceOrder();

  void saveWeatherRegion(String region);
  String getWeatherRegion();

  void saveWeatherTown(String town);
  String getWeatherTown();
};

extern ConfigManager configManager;
