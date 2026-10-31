#pragma once

#include <memory>
#include <vector>

#include "Preferences.h"

#define CONFIG_NAMESPACE "butler-config"

// System
#define CONFIG_BRIGHTNESS "brightness"

// Bus
#define CONFIG_BUS_API_KEY "bus-api-key"
#define CONFIG_BUS_STOP_CODE "busstop-code"  // legacy single stop
#define CONFIG_BUS_STOPS "bus-stops"         // JSON list of stops
#define CONFIG_BUS_STOP_INDEX "bus-stop-idx"
#define CONFIG_BUS_SERVICE_ORDER "bus-svc-order"

// Weathers
#define CONFIG_WEATHER_REGION "weather-region"
#define CONFIG_WEATHER_TOWN "weather-town"

// Display (keys must stay <= 15 chars)
#define CONFIG_DIM_IDLE_ON "dim-idle-on"
#define CONFIG_DIM_IDLE_MIN "dim-idle-min"
#define CONFIG_DIM_IDLE_LVL "dim-idle-lvl"
#define CONFIG_NIGHT_ON "night-on"
#define CONFIG_NIGHT_START "night-start"
#define CONFIG_NIGHT_END "night-end"
#define CONFIG_NIGHT_LVL "night-lvl"
#define CONFIG_NIGHT_THEME "night-theme"

#define MAX_BUS_STOPS 8

using namespace std;

struct BusServiceOrderConfig {
  unique_ptr<String[]> services;
  int size;
};

struct BusStopConfig {
  String code;
  String name;  // may be empty until looked up / named in the portal
};

struct DisplayConfig {
  uint8_t brightness;      // 0-255, user brightness
  bool idleDimEnabled;     // dim after no touch for idleDimMinutes
  uint8_t idleDimMinutes;  // 1-120
  uint8_t idleDimLevel;    // 0-255 brightness while idle-dimmed
  bool nightEnabled;       // dim during the night window
  bool nightTheme;         // warm (no blue) colours during the night window
  uint16_t nightStart;     // minutes after midnight
  uint16_t nightEnd;       // minutes after midnight
  uint8_t nightLevel;      // 0-255 brightness during the night window
};

/*
  All reads and writes go through one mutex: the web server, the network
  worker and the UI loop run on different FreeRTOS tasks and the shared
  Preferences handle is not safe to use concurrently.
*/
class ConfigManager {
 private:
  Preferences preferences;
  void lock();
  void unlock();
  volatile uint32_t stopsRevision = 1;
  volatile uint32_t displayRevision = 1;

 public:
  ConfigManager();

  void saveLtaDatamallApiKey(String apiKey);
  String getLtaDatamallApiKey();

  void saveBrightness(uint8_t brightness);
  uint8_t getBrightness();

  // Legacy single-stop accessors, mapped onto the stop list.
  void saveBusStopCode(String busStopCode);
  String getBusStopCode();

  vector<BusStopConfig> getBusStops();
  void saveBusStops(const vector<BusStopConfig>& stops);
  void setBusStopName(const String& code, const String& name);
  uint8_t getActiveBusStopIndex();
  void saveActiveBusStopIndex(uint8_t index);
  // Bumped on every stop list change so screens can refresh cheaply.
  uint32_t getStopsRevision() { return stopsRevision; }

  // Stored as comma separated, <length>,<svc 1>,<svc 2>,...
  void saveBusServiceOrder(String serviceOrder[], int size);
  BusServiceOrderConfig getBusServiceOrder();

  void saveWeatherRegion(String region);
  String getWeatherRegion();

  void saveWeatherTown(String town);
  String getWeatherTown();

  DisplayConfig getDisplayConfig();
  void saveDisplayConfig(const DisplayConfig& config);
  uint32_t getDisplayRevision() { return displayRevision; }
};

extern ConfigManager configManager;
