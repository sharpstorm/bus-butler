#include "WeatherCacheMutator.h"

#include "../Util/Time.h"
#include "NetworkDataCache.h"

struct WeatherQueryDto {
  NeaWeatherClient* client;
  bool queryTwoHour;
  bool queryTwentyFourHour;
  int32_t* lastWeatherApiTimestampPtr;
  bool* isWeatherFetchInProgressPtr;
};

bool WeatherCacheMutator::isStale() {
  long timeNow = TimeUtil::getUnixTime();
  return timeNow - lastWeatherApiTimestamp > WEATHER_REFRESH_INTERVAL_SECS;
}

bool WeatherCacheMutator::shouldFetch() {
  return isStale() && !isWeatherFetchInProgress;
}

void WeatherCacheMutator::invalidateCache() { lastWeatherApiTimestamp = -1; }

void WeatherCacheMutator::disable() { lastWeatherApiTimestamp = INT32_MAX; }

int32_t WeatherCacheMutator::getLastApiCallTimestamp() {
  return lastWeatherApiTimestamp;
}

void WeatherCacheMutator::queryWeatherWorker(void* pvParameters) {
  WeatherQueryDto* dtoPtr = (WeatherQueryDto*)(pvParameters);

  if (dtoPtr->queryTwoHour) {
    networkDataCache.cachedTwoHourForecast =
        dtoPtr->client->getTwoHourForecast();
  }

  if (dtoPtr->queryTwentyFourHour) {
    networkDataCache.cachedTwentyFourHourForecast =
        dtoPtr->client->getTwentyFourHourForecast();
  }
  *dtoPtr->lastWeatherApiTimestampPtr = TimeUtil::getUnixTime();
  *dtoPtr->isWeatherFetchInProgressPtr = false;

  delete dtoPtr;
  vTaskDelete(NULL);
}

bool WeatherCacheMutator::updateCache() {
  auto taskParams =
      new WeatherQueryDto{&neaWeatherClient, true, true,
                          &lastWeatherApiTimestamp, &isWeatherFetchInProgress};
  isWeatherFetchInProgress = true;
  TaskHandle_t taskHandle = NULL;
  BaseType_t taskCreated = xTaskCreate(queryWeatherWorker, "homescreen-weather",
                                       8192, taskParams, 1, &taskHandle);

  if (taskCreated != pdPASS) {
    Serial.println("Failed to create weather query task");
    delete taskParams;
    isWeatherFetchInProgress = false;
    lastWeatherApiTimestamp = TimeUtil::getUnixTime();
    return false;
  }
  return true;
}

WeatherCacheMutator weatherQuery;
