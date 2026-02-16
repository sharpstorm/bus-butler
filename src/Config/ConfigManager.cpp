#include "ConfigManager.h"

#include "../Util/MemoryPolyfill.h"

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

void ConfigManager::saveBusServiceOrder(String services[], int size) {
  // Stored as comma separated, <length>,<svc 1>,<svc 2>,...
  String orderString = String(size);
  for (int i = 0; i < size; i++) {
    orderString += "," + services[i];
  }

  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_BUS_SERVICE_ORDER, orderString);
  preferences.end();
}

BusServiceOrderConfig ConfigManager::getBusServiceOrder() {
  preferences.begin(CONFIG_NAMESPACE, true);
  String order = preferences.getString(CONFIG_BUS_SERVICE_ORDER, "");
  preferences.end();

  if (order.length() == 0) {
    return BusServiceOrderConfig{nullptr, 0};
  }

  int i = 0;
  for (; i < order.length(); i++) {
    if (order.charAt(i) == ',') {
      break;
    }
  }

  int size = atoi(order.substring(0, i).c_str());
  unique_ptr<String[]> ret(new String[size]);

  i++;
  int start = i;
  int retCtr = 0;
  for (; i < order.length(); i++) {
    if (order.charAt(i) == ',') {
      ret[retCtr] = order.substring(start, i);
      retCtr++;
      start = i + 1;
    }
  }
  ret[retCtr] = order.substring(start, i);

  return BusServiceOrderConfig{move(ret), size};
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
