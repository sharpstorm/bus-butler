#pragma once

#include <memory>

#include "../NeaWeatherApi/NeaWeatherApiClient.h"
#include "../NeaWeatherApi/NeaWeatherApiTypes.h"

using namespace std;

class WeatherCacheMutator {
 private:
  int32_t lastWeatherApiTimestamp = -1;
  bool isWeatherFetchInProgress = false;

  static void queryWeatherWorker(void* pvParameters);

  // Weather refresh interval: 5 minutes
  static constexpr int32_t WEATHER_REFRESH_INTERVAL_SECS = 300;

 public:
  bool isStale();
  bool shouldFetch();

  void invalidateCache();
  void disable();

  int32_t getLastApiCallTimestamp();

  /* Returns if successful*/
  bool updateCache();
};

extern WeatherCacheMutator weatherQuery;
