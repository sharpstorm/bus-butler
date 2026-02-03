#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

void ConfigManager::saveLtaDatamallApiKey(String apiKey) {
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_BUS_API_KEY, apiKey);
  preferences.end();
}

String ConfigManager::getLtaDatamallApiKey() {
  preferences.begin(CONFIG_NAMESPACE, true);
  String apiKey = preferences.getString(CONFIG_BUS_API_KEY, "");
  preferences.end();

  return apiKey;
}

void ConfigManager::saveBrightness(uint8_t brightness) {
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putChar(CONFIG_BRIGHTNESS, brightness);
  preferences.end();
}

uint8_t ConfigManager::getBrightness() {
  preferences.begin(CONFIG_NAMESPACE, true);
  uint8_t brightness = preferences.getChar(CONFIG_BRIGHTNESS, 255);
  preferences.end();

  return brightness;
}

void ConfigManager::saveBusStopCode(String busStopCode) {
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_BUS_STOP_CODE, busStopCode);
  preferences.end();
}

String ConfigManager::getBusStopCode() {
  preferences.begin(CONFIG_NAMESPACE, true);
  String apiKey = preferences.getString(CONFIG_BUS_STOP_CODE, "");
  preferences.end();

  return apiKey;
}

void ConfigManager::saveWeatherRegion(String region) {
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_WEATHER_REGION, region);
  preferences.end();
}

String ConfigManager::getWeatherRegion() {
  preferences.begin(CONFIG_NAMESPACE, true);
  String region = preferences.getString(CONFIG_WEATHER_REGION, "");
  preferences.end();
  return region;
}

void ConfigManager::saveWeatherTown(String town) {
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_WEATHER_TOWN, town);
  preferences.end();
}

String ConfigManager::getWeatherTown() {
  preferences.begin(CONFIG_NAMESPACE, true);
  String town = preferences.getString(CONFIG_WEATHER_TOWN, "");
  preferences.end();
  return town;
}

ConfigManager configManager;
