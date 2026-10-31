#include "ConfigManager.h"

#include <ArduinoJson.h>

#include "../Util/MemoryPolyfill.h"

static SemaphoreHandle_t configMutex = nullptr;

ConfigManager::ConfigManager() {}

void ConfigManager::lock() {
  if (configMutex == nullptr) {
    configMutex = xSemaphoreCreateRecursiveMutex();
  }
  xSemaphoreTakeRecursive(configMutex, portMAX_DELAY);
}

void ConfigManager::unlock() { xSemaphoreGiveRecursive(configMutex); }

void ConfigManager::saveLtaDatamallApiKey(String apiKey) {
  lock();
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_BUS_API_KEY, apiKey);
  preferences.end();
  unlock();
}

String ConfigManager::getLtaDatamallApiKey() {
  lock();
  preferences.begin(CONFIG_NAMESPACE, true);
  String apiKey = preferences.getString(CONFIG_BUS_API_KEY, "");
  preferences.end();
  unlock();
  return apiKey;
}

void ConfigManager::saveBrightness(uint8_t brightness) {
  lock();
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putChar(CONFIG_BRIGHTNESS, (int8_t)brightness);
  preferences.end();
  displayRevision++;
  unlock();
}

uint8_t ConfigManager::getBrightness() {
  lock();
  preferences.begin(CONFIG_NAMESPACE, true);
  // Stored as a signed char by older firmware; keep the type.
  uint8_t brightness = (uint8_t)preferences.getChar(CONFIG_BRIGHTNESS, (int8_t)255);
  preferences.end();
  unlock();
  return brightness;
}

// ---------------------------------------------------------------------------
// Bus stops

vector<BusStopConfig> ConfigManager::getBusStops() {
  lock();
  preferences.begin(CONFIG_NAMESPACE, true);
  String raw = preferences.getString(CONFIG_BUS_STOPS, "");
  String legacy = preferences.getString(CONFIG_BUS_STOP_CODE, "");
  preferences.end();
  unlock();

  vector<BusStopConfig> stops;
  if (raw.length() > 0) {
    JsonDocument doc;
    if (!deserializeJson(doc, raw) && doc.is<JsonArray>()) {
      for (JsonObject item : doc.as<JsonArray>()) {
        String code = item["c"] | "";
        if (code.length() == 0) continue;
        stops.push_back({code, item["n"] | ""});
        if (stops.size() >= MAX_BUS_STOPS) break;
      }
    }
  }

  // Migrate the single stop saved by older firmware.
  if (stops.empty() && legacy.length() > 0) {
    stops.push_back({legacy, ""});
  }
  return stops;
}

void ConfigManager::saveBusStops(const vector<BusStopConfig>& stops) {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (size_t i = 0; i < stops.size() && i < MAX_BUS_STOPS; i++) {
    JsonObject item = arr.add<JsonObject>();
    item["c"] = stops[i].code;
    item["n"] = stops[i].name;
  }
  String raw;
  serializeJson(doc, raw);

  lock();
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_BUS_STOPS, raw);
  // Keep the legacy key pointing at the first stop for older tools.
  preferences.putString(CONFIG_BUS_STOP_CODE,
                        stops.empty() ? String("") : stops[0].code);
  uint8_t index = preferences.getUChar(CONFIG_BUS_STOP_INDEX, 0);
  if (index >= stops.size()) {
    preferences.putUChar(CONFIG_BUS_STOP_INDEX, 0);
  }
  preferences.end();
  stopsRevision++;
  unlock();
}

void ConfigManager::setBusStopName(const String& code, const String& name) {
  lock();
  vector<BusStopConfig> stops = getBusStops();
  bool changed = false;
  for (auto& stop : stops) {
    if (stop.code == code && stop.name != name) {
      stop.name = name;
      changed = true;
    }
  }
  if (changed) {
    saveBusStops(stops);
  }
  unlock();
}

uint8_t ConfigManager::getActiveBusStopIndex() {
  lock();
  preferences.begin(CONFIG_NAMESPACE, true);
  uint8_t index = preferences.getUChar(CONFIG_BUS_STOP_INDEX, 0);
  preferences.end();
  unlock();
  return index;
}

void ConfigManager::saveActiveBusStopIndex(uint8_t index) {
  lock();
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putUChar(CONFIG_BUS_STOP_INDEX, index);
  preferences.end();
  unlock();
}

void ConfigManager::saveBusStopCode(String busStopCode) {
  lock();
  vector<BusStopConfig> stops = getBusStops();
  uint8_t index = getActiveBusStopIndex();
  if (stops.empty()) {
    stops.push_back({busStopCode, ""});
  } else {
    if (index >= stops.size()) index = 0;
    if (stops[index].code != busStopCode) {
      stops[index] = {busStopCode, ""};
    }
  }
  saveBusStops(stops);
  unlock();
}

String ConfigManager::getBusStopCode() {
  vector<BusStopConfig> stops = getBusStops();
  if (stops.empty()) return "";
  uint8_t index = getActiveBusStopIndex();
  if (index >= stops.size()) index = 0;
  return stops[index].code;
}

// ---------------------------------------------------------------------------
// Bus service order

void ConfigManager::saveBusServiceOrder(String services[], int size) {
  // Stored as comma separated, <length>,<svc 1>,<svc 2>,...
  String orderString = String(size);
  for (int i = 0; i < size; i++) {
    orderString += "," + services[i];
  }

  lock();
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_BUS_SERVICE_ORDER, orderString);
  preferences.end();
  stopsRevision++;
  unlock();
}

BusServiceOrderConfig ConfigManager::getBusServiceOrder() {
  lock();
  preferences.begin(CONFIG_NAMESPACE, true);
  String order = preferences.getString(CONFIG_BUS_SERVICE_ORDER, "");
  preferences.end();
  unlock();

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
  if (size <= 0) {
    return BusServiceOrderConfig{nullptr, 0};
  }
  unique_ptr<String[]> ret(new String[size]);

  i++;
  int start = i;
  int retCtr = 0;
  for (; i < order.length() && retCtr < size; i++) {
    if (order.charAt(i) == ',') {
      ret[retCtr] = order.substring(start, i);
      retCtr++;
      start = i + 1;
    }
  }
  if (retCtr < size) {
    ret[retCtr] = order.substring(start, i);
  }

  return BusServiceOrderConfig{move(ret), size};
}

// ---------------------------------------------------------------------------
// Weather

void ConfigManager::saveWeatherRegion(String region) {
  lock();
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_WEATHER_REGION, region);
  preferences.end();
  unlock();
}

String ConfigManager::getWeatherRegion() {
  lock();
  preferences.begin(CONFIG_NAMESPACE, true);
  String region = preferences.getString(CONFIG_WEATHER_REGION, "");
  preferences.end();
  unlock();
  return region;
}

void ConfigManager::saveWeatherTown(String town) {
  lock();
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_WEATHER_TOWN, town);
  preferences.end();
  unlock();
}

String ConfigManager::getWeatherTown() {
  lock();
  preferences.begin(CONFIG_NAMESPACE, true);
  String town = preferences.getString(CONFIG_WEATHER_TOWN, "");
  preferences.end();
  unlock();
  return town;
}

// ---------------------------------------------------------------------------
// Display

DisplayConfig ConfigManager::getDisplayConfig() {
  lock();
  preferences.begin(CONFIG_NAMESPACE, true);
  DisplayConfig config;
  config.brightness =
      (uint8_t)preferences.getChar(CONFIG_BRIGHTNESS, (int8_t)255);
  config.idleDimEnabled = preferences.getBool(CONFIG_DIM_IDLE_ON, false);
  config.idleDimMinutes = preferences.getUChar(CONFIG_DIM_IDLE_MIN, 5);
  config.idleDimLevel = preferences.getUChar(CONFIG_DIM_IDLE_LVL, 20);
  config.nightEnabled = preferences.getBool(CONFIG_NIGHT_ON, false);
  config.nightTheme = preferences.getBool(CONFIG_NIGHT_THEME, false);
  config.nightStart = preferences.getUShort(CONFIG_NIGHT_START, 23 * 60);
  config.nightEnd = preferences.getUShort(CONFIG_NIGHT_END, 7 * 60);
  config.nightLevel = preferences.getUChar(CONFIG_NIGHT_LVL, 10);
  preferences.end();
  unlock();

  if (config.idleDimMinutes == 0) config.idleDimMinutes = 1;
  if (config.nightStart >= 24 * 60) config.nightStart = 23 * 60;
  if (config.nightEnd >= 24 * 60) config.nightEnd = 7 * 60;
  return config;
}

void ConfigManager::saveDisplayConfig(const DisplayConfig& config) {
  lock();
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putChar(CONFIG_BRIGHTNESS, (int8_t)config.brightness);
  preferences.putBool(CONFIG_DIM_IDLE_ON, config.idleDimEnabled);
  preferences.putUChar(CONFIG_DIM_IDLE_MIN,
                       constrain(config.idleDimMinutes, 1, 120));
  preferences.putUChar(CONFIG_DIM_IDLE_LVL, config.idleDimLevel);
  preferences.putBool(CONFIG_NIGHT_ON, config.nightEnabled);
  preferences.putBool(CONFIG_NIGHT_THEME, config.nightTheme);
  preferences.putUShort(CONFIG_NIGHT_START, config.nightStart % (24 * 60));
  preferences.putUShort(CONFIG_NIGHT_END, config.nightEnd % (24 * 60));
  preferences.putUChar(CONFIG_NIGHT_LVL, config.nightLevel);
  preferences.end();
  displayRevision++;
  unlock();
}

ConfigManager configManager;
