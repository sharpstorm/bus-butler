#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

void ConfigManager::saveLtaDatamallApiKey(String apiKey) {
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_BUS_API_KEY, apiKey);
  preferences.end();
}

String ConfigManager::getLtaDatamallApiKey() {
  preferences.begin(CONFIG_NAMESPACE, false);
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
  preferences.begin(CONFIG_NAMESPACE, false);
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
  preferences.begin(CONFIG_NAMESPACE, false);
  String apiKey = preferences.getString(CONFIG_BUS_STOP_CODE, "");
  preferences.end();

  return apiKey;
}

ConfigManager configManager;
