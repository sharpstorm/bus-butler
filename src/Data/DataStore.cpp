#include "DataStore.h"

#include <WiFi.h>

#include "../Config/ConfigManager.h"
#include "../LtaDatamallClient/LtaDatamallClient.h"
#include "../NeaWeatherApi/NeaWeatherApiClient.h"
#include "../Util/MemoryPolyfill.h"
#include "../Wifi/WifiManager.h"

#define DATA_TASK_STACK 16384

static SemaphoreHandle_t dataMutex = nullptr;

// Owned by the worker task only.
static LtaDatamallClient* ltaClient = nullptr;
static NeaWeatherClient* neaClient = nullptr;

void DataStore::begin() {
  if (dataMutex == nullptr) {
    dataMutex = xSemaphoreCreateRecursiveMutex();
  }
  if (task != nullptr) {
    return;
  }

  vector<BusStopConfig> stops = configManager.getBusStops();
  uint8_t index = configManager.getActiveBusStopIndex();
  if (!stops.empty()) {
    setActiveStop(stops[index < stops.size() ? index : 0].code);
  }

  xTaskCreatePinnedToCore(taskMain, "data-worker", DATA_TASK_STACK, this, 1,
                          &task, 0);
}

void DataStore::lock() {
  if (dataMutex == nullptr) {
    dataMutex = xSemaphoreCreateRecursiveMutex();
  }
  xSemaphoreTakeRecursive(dataMutex, portMAX_DELAY);
}

void DataStore::unlock() { xSemaphoreGiveRecursive(dataMutex); }

void DataStore::setActiveStop(const String& code) {
  lock();
  if (code != activeStop) {
    activeStop = code;
    // Drop the old stop's arrivals right away so they never show under the
    // new stop's name.
    busArrival.reset();
    busArrivalStop = "";
    busStatus = SourceStatus();
    busRequested = true;
    version++;
  }
  unlock();
}

String DataStore::getActiveStop() {
  lock();
  String code = activeStop;
  unlock();
  return code;
}

void DataStore::requestRefresh() {
  busRequested = true;
  refreshAllRequested = true;
}

void DataStore::requestStopLookup(const String& code) {
  lock();
  if (!(lookupCode == code && (lookupState == StopLookupState::Pending ||
                               lookupState == StopLookupState::Found))) {
    lookupCode = code;
    lookupName = "";
    lookupState = StopLookupState::Pending;
    lookupRequested = true;
  }
  unlock();
}

StopLookupState DataStore::getStopLookup(const String& code, String& name) {
  lock();
  StopLookupState state =
      code == lookupCode ? lookupState : StopLookupState::Unknown;
  name = lookupName;
  unlock();
  return state;
}

void DataStore::taskMain(void* param) {
  DataStore* self = static_cast<DataStore*>(param);
  ltaClient = new LtaDatamallClient();
  neaClient = new NeaWeatherClient();
  while (true) {
    self->loopOnce();
  }
}

bool DataStore::due(const SourceStatus& status, uint32_t refreshMs,
                    uint32_t retryMs) {
  if (!status.attempted) return true;
  uint32_t wait = status.ok ? refreshMs : retryMs;
  return millis() - status.lastAttemptMs >= wait;
}

void DataStore::markAttempt(SourceStatus& status, bool ok, int error) {
  status.attempted = true;
  status.lastAttemptMs = millis();
  status.ok = ok;
  status.lastError = error;
  if (ok) {
    status.lastSuccessMs = status.lastAttemptMs;
  }
}

void DataStore::loopOnce() {
  if (!WiFi.isConnected()) {
    if (millis() - lastWifiReconnectMs > WIFI_RECONNECT_MS) {
      lastWifiReconnectMs = millis();
      wifiManager.reconnectIfNeeded();
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    return;
  }

  if (refreshAllRequested) {
    refreshAllRequested = false;
    lock();
    trainStatus.attempted = false;
    weatherStatus.attempted = false;
    extrasStatus.attempted = false;
    unlock();
  }

  // Pick up a changed API key without hitting flash on every pass
  if (millis() - lastKeyCheckMs > 10000 || lastKeyCheckMs == 0) {
    lastKeyCheckMs = millis();
    ltaClient->reconfigure();
  }

  // One job per pass, most time-sensitive first, so a stop change is never
  // stuck behind a slow weather fetch for long.
  if (busRequested || due(busStatus, BUS_REFRESH_MS, BUS_RETRY_MS)) {
    fetchBus();
  } else if (runPendingLookup()) {
  } else if (due(trainStatus, TRAIN_REFRESH_MS, TRAIN_RETRY_MS)) {
    fetchTrain();
  } else if (due(weatherStatus, WEATHER_REFRESH_MS, WEATHER_RETRY_MS)) {
    fetchWeather();
  } else if (due(extrasStatus, EXTRAS_REFRESH_MS, EXTRAS_RETRY_MS)) {
    fetchExtras();
  } else if (autoNameNextStop()) {
  } else {
    vTaskDelay(pdMS_TO_TICKS(250));
    return;
  }
  vTaskDelay(pdMS_TO_TICKS(20));
}

void DataStore::fetchBus() {
  busRequested = false;
  String stop = getActiveStop();
  if (stop.length() == 0 || !ltaClient->isConfigured()) {
    lock();
    markAttempt(busStatus, false, stop.length() == 0 ? -10 : -11);
    version++;
    unlock();
    return;
  }

  unique_ptr<BusArrivalApiResponse> resp = ltaClient->getBusArrival(stop);

  lock();
  if (stop == activeStop) {  // Ignore results for a stop we switched away from
    markAttempt(busStatus, resp != nullptr, resp != nullptr ? 200 : -1);
    if (resp != nullptr) {
      busArrival = move(resp);
      busArrivalStop = stop;
    }
    version++;
  }
  unlock();
}

void DataStore::fetchTrain() {
  if (!ltaClient->isConfigured()) {
    lock();
    markAttempt(trainStatus, false, -11);
    unlock();
    return;
  }

  unique_ptr<TrainAlerts> resp = ltaClient->getTrainServiceAlerts();
  lock();
  markAttempt(trainStatus, resp != nullptr, resp != nullptr ? 200 : -1);
  if (resp != nullptr) {
    trainAlerts = move(resp);
  }
  version++;
  unlock();
}

// The configured town's coordinates, from the 2-hour forecast metadata.
void DataStore::townLocation(float& lat, float& lon) {
  String town = configManager.getWeatherTown();
  lat = 1.3521;  // Singapore centre as a fallback
  lon = 103.8198;
  lock();
  if (twoHour != nullptr) {
    for (const auto& area : twoHour->forecasts) {
      if (area.area.equalsIgnoreCase(town) && area.latitude != 0) {
        lat = area.latitude;
        lon = area.longitude;
        break;
      }
    }
  }
  unlock();
}

static float distanceKm(float lat1, float lon1, float lat2, float lon2) {
  float dLat = (lat2 - lat1) * 111.32f;
  float dLon = (lon2 - lon1) * 111.32f * cosf(lat1 * DEG_TO_RAD);
  return sqrtf(dLat * dLat + dLon * dLon);
}

// Forecasts plus rainfall: rainfall updates every 5 minutes and drives the
// heavy rain banner, so it rides on the 5-minute forecast job.
void DataStore::fetchWeather() {
  unique_ptr<TwoHourForecastResponse> two = neaClient->getTwoHourForecast();
  unique_ptr<TwentyFourHourForecastResponse> day =
      neaClient->getTwentyFourHourForecast();

  lock();
  bool ok = two != nullptr || day != nullptr;
  markAttempt(weatherStatus, ok, ok ? 200 : -1);
  if (two != nullptr) twoHour = move(two);
  if (day != nullptr) twentyFourHour = move(day);
  version++;
  unlock();

  float lat, lon;
  townLocation(lat, lon);
  unique_ptr<StationReadingsResponse> rain =
      neaClient->getStationReadings(RAINFALL_PATH);
  if (rain == nullptr) return;  // Keep the last readings

  const StationReading* nearest = rain->nearest(lat, lon);
  const StationReading* heaviest = nullptr;
  float heaviestKm = 0;
  for (const auto& station : rain->stations) {
    if (!station.hasValue) continue;
    float km = distanceKm(lat, lon, station.latitude, station.longitude);
    if (km > HEAVY_RAIN_RADIUS_KM) continue;
    if (heaviest == nullptr || station.value > heaviest->value) {
      heaviest = &station;
      heaviestKm = km;
    }
  }

  lock();
  weatherNow.hasRainfall = nearest != nullptr;
  if (nearest != nullptr) {
    weatherNow.rainfall = nearest->value;
    weatherNow.rainfallStation = nearest->name;
  }
  weatherNow.heavyRainNearby =
      heaviest != nullptr && heaviest->value >= HEAVY_RAIN_MM_PER_5MIN;
  weatherNow.nearbyRainRate = heaviest != nullptr ? heaviest->value * 12 : 0;
  weatherNow.nearbyRainKm = heaviestKm;
  weatherNow.nearbyRainStation = heaviest != nullptr ? heaviest->name : "";
  version++;
  unlock();
}

void DataStore::fetchExtras() {
  String region = configManager.getWeatherRegion();
  float lat, lon;
  townLocation(lat, lon);

  unique_ptr<FourDayOutlookResponse> outlook = neaClient->getFourDayOutlook();
  unique_ptr<StationReadingsResponse> temp =
      neaClient->getStationReadings(AIR_TEMPERATURE_PATH);
  unique_ptr<StationReadingsResponse> humidity =
      neaClient->getStationReadings(RELATIVE_HUMIDITY_PATH);
  unique_ptr<PsiResponse> psi = neaClient->getPsi();
  unique_ptr<UvResponse> uv = neaClient->getUv();

  WeatherNow now;
  if (temp != nullptr) {
    const StationReading* s = temp->nearest(lat, lon);
    if (s != nullptr) {
      now.hasAirTemp = true;
      now.airTemp = s->value;
      now.airTempStation = s->name;
    }
  }
  if (humidity != nullptr) {
    const StationReading* s = humidity->nearest(lat, lon);
    if (s != nullptr) {
      now.hasHumidity = true;
      now.humidity = s->value;
    }
  }
  if (psi != nullptr) now.psi = psi->forRegion(region);
  if (uv != nullptr) now.uv = uv->value;

  lock();
  bool ok = outlook != nullptr || temp != nullptr || psi != nullptr;
  markAttempt(extrasStatus, ok, ok ? 200 : -1);
  if (outlook != nullptr) fourDay = move(outlook);
  // Keep the previous value of anything that failed this round.
  if (!now.hasAirTemp && weatherNow.hasAirTemp) {
    now.hasAirTemp = true;
    now.airTemp = weatherNow.airTemp;
    now.airTempStation = weatherNow.airTempStation;
  }
  if (!now.hasHumidity && weatherNow.hasHumidity) {
    now.hasHumidity = true;
    now.humidity = weatherNow.humidity;
  }
  if (now.psi < 0) now.psi = weatherNow.psi;
  if (now.uv < 0) now.uv = weatherNow.uv;
  // Rainfall belongs to fetchWeather; carry it over unchanged.
  now.hasRainfall = weatherNow.hasRainfall;
  now.rainfall = weatherNow.rainfall;
  now.rainfallStation = weatherNow.rainfallStation;
  now.heavyRainNearby = weatherNow.heavyRainNearby;
  now.nearbyRainRate = weatherNow.nearbyRainRate;
  now.nearbyRainKm = weatherNow.nearbyRainKm;
  now.nearbyRainStation = weatherNow.nearbyRainStation;
  weatherNow = now;
  version++;
  unlock();
}

bool DataStore::runPendingLookup() {
  lock();
  if (!lookupRequested) {
    unlock();
    return false;
  }
  lookupRequested = false;
  String code = lookupCode;
  unlock();

  String name;
  int result = ltaClient->isConfigured()
                   ? ltaClient->lookupBusStopName(code, name)
                   : -1;

  lock();
  if (code == lookupCode) {
    lookupName = name;
    lookupState = result == 1   ? StopLookupState::Found
                  : result == 0 ? StopLookupState::NotFound
                                : StopLookupState::Error;
  }
  unlock();
  return true;
}

// Fill in names for saved stops that don't have one yet (e.g. migrated from
// the single-stop setting), one stop per pass.
bool DataStore::autoNameNextStop() {
  if (!ltaClient->isConfigured()) return false;

  vector<BusStopConfig> stops = configManager.getBusStops();
  for (const auto& stop : stops) {
    if (stop.name.length() > 0) continue;
    String marker = "|" + stop.code + "|";
    if (autoNamedCodes.indexOf(marker) >= 0) continue;
    autoNamedCodes += marker;

    String name;
    if (ltaClient->lookupBusStopName(stop.code, name) == 1) {
      configManager.setBusStopName(stop.code, name);
      lock();
      version++;
      unlock();
    }
    return true;
  }
  return false;
}

DataStore dataStore;
