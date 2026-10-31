#pragma once

#include <Arduino.h>

#include <memory>

#include "../LtaDatamallClient/BusArrivalApiTypes.h"
#include "../LtaDatamallClient/TrainAlertTypes.h"
#include "../NeaWeatherApi/NeaWeatherApiTypes.h"

using namespace std;

#define BUS_REFRESH_MS (30 * 1000)
#define BUS_RETRY_MS (15 * 1000)
#define TRAIN_REFRESH_MS (2 * 60 * 1000)
#define TRAIN_RETRY_MS (60 * 1000)
#define WEATHER_REFRESH_MS (5 * 60 * 1000)
#define WEATHER_RETRY_MS (60 * 1000)
#define EXTRAS_REFRESH_MS (10 * 60 * 1000)
#define EXTRAS_RETRY_MS (2 * 60 * 1000)
#define WIFI_RECONNECT_MS (30 * 1000)

// Heavy rain: >= 10 mm/h (0.8 mm in a 5-minute reading) within 4 km
#define HEAVY_RAIN_MM_PER_5MIN 0.8f
#define HEAVY_RAIN_RADIUS_KM 4.0f

// Health of one data source, shown on the status page.
struct SourceStatus {
  uint32_t lastAttemptMs = 0;
  uint32_t lastSuccessMs = 0;
  bool attempted = false;
  bool ok = false;
  int lastError = 0;  // HTTP status or negative error
};

// Readings from the weather station nearest to the configured town.
struct WeatherNow {
  bool hasAirTemp = false;
  float airTemp = 0;
  String airTempStation;
  bool hasHumidity = false;
  float humidity = 0;
  bool hasRainfall = false;
  float rainfall = 0;  // mm in the last 5 minutes at the nearest station
  String rainfallStation;
  // Heaviest 5-minute rainfall at any station within HEAVY_RAIN_RADIUS_KM
  bool heavyRainNearby = false;
  float nearbyRainRate = 0;  // mm/h equivalent
  float nearbyRainKm = 0;
  String nearbyRainStation;
  int psi = -1;  // 24h PSI for the configured region
  int uv = -1;   // latest hourly UV index
};

enum class StopLookupState { Unknown, Pending, Found, NotFound, Error };

/*
  Owns all network data and the single worker task that fetches it.

  Every fetch runs on the worker task, one at a time, so only one TLS session
  is ever open (they are expensive in internal RAM) and API clients are never
  shared between tasks. Readers take the lock, read, and release it quickly;
  getVersion() changes whenever any data changes so screens know to redraw.
*/
class DataStore {
 public:
  void begin();
  bool isStarted() { return task != nullptr; }

  void lock();
  void unlock();
  uint32_t getVersion() { return version; }

  // ---- Data, read under lock() ----
  unique_ptr<BusArrivalApiResponse> busArrival;
  String busArrivalStop;  // stop code busArrival belongs to
  unique_ptr<TwoHourForecastResponse> twoHour;
  unique_ptr<TwentyFourHourForecastResponse> twentyFourHour;
  unique_ptr<FourDayOutlookResponse> fourDay;
  WeatherNow weatherNow;
  unique_ptr<TrainAlerts> trainAlerts;

  SourceStatus busStatus;
  SourceStatus trainStatus;
  SourceStatus weatherStatus;
  SourceStatus extrasStatus;

  // ---- Requests, safe from any task ----
  void setActiveStop(const String& code);
  String getActiveStop();
  void requestRefresh();

  void requestStopLookup(const String& code);
  StopLookupState getStopLookup(const String& code, String& name);

 private:
  TaskHandle_t task = nullptr;
  volatile uint32_t version = 0;

  String activeStop;
  volatile bool busRequested = true;
  volatile bool refreshAllRequested = false;

  String lookupCode;
  String lookupName;
  StopLookupState lookupState = StopLookupState::Unknown;
  bool lookupRequested = false;
  String autoNamedCodes;  // codes already auto-looked-up this boot

  uint32_t lastWifiReconnectMs = 0;
  uint32_t lastKeyCheckMs = 0;

  static void taskMain(void* param);
  void loopOnce();
  bool due(const SourceStatus& status, uint32_t refreshMs, uint32_t retryMs);
  void markAttempt(SourceStatus& status, bool ok, int error);

  void townLocation(float& lat, float& lon);
  void fetchBus();
  void fetchTrain();
  void fetchWeather();
  void fetchExtras();
  bool runPendingLookup();
  bool autoNameNextStop();
};

extern DataStore dataStore;
