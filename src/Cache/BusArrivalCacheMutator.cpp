#include "BusArrivalCacheMutator.h"

#include "../Util/Time.h"
#include "./NetworkDataCache.h"

using namespace std;

struct QueryBusArrivalWorkerDto {
  String busStopCode;
  int32_t* lastBusArrivalApiTimestampPtr;
  bool* isBusArrivalFetchInProgressPtr;
  LtaDatamallClient* client;
};

bool BusArrivalCacheMutator::isStale() {
  long timeNow = TimeUtil::getUnixTime();
  return timeNow - lastBusArrivalApiTimestamp >
         BUS_ARRIVAL_REFRESH_INTERVAL_SECS;
}

bool BusArrivalCacheMutator::shouldFetch() {
  return isStale() && !isBusArrivalFetchInProgress;
}

void BusArrivalCacheMutator::invalidateCache() {
  lastBusArrivalApiTimestamp = -1;
}

void BusArrivalCacheMutator::disable() {
  lastBusArrivalApiTimestamp = INT32_MAX;
}

int32_t BusArrivalCacheMutator::getLastApiCallTimestamp() {
  return lastBusArrivalApiTimestamp;
}

void BusArrivalCacheMutator::queryBusArrivalWorker(void* pvParameters) {
  QueryBusArrivalWorkerDto* dtoPtr = (QueryBusArrivalWorkerDto*)(pvParameters);
  networkDataCache.cachedBusArrival =
      dtoPtr->client->getBusArrival(dtoPtr->busStopCode);
  *dtoPtr->lastBusArrivalApiTimestampPtr = TimeUtil::getUnixTime();
  *dtoPtr->isBusArrivalFetchInProgressPtr = false;

  delete dtoPtr;
  vTaskDelete(NULL);
}

bool BusArrivalCacheMutator::updateCache(String busStopCode) {
  TaskHandle_t taskRef = NULL;
  if (isBusArrivalFetchInProgress) {
    return true;
  }

  isBusArrivalFetchInProgress = true;
  auto taskParams = new QueryBusArrivalWorkerDto{
      busStopCode, &lastBusArrivalApiTimestamp, &isBusArrivalFetchInProgress,
      &ltaDatamallClient};
  TaskHandle_t taskHandle = NULL;
  BaseType_t taskCreated = xTaskCreate(queryBusArrivalWorker, "homescreen-bus",
                                       8192, taskParams, 1, &taskHandle);

  if (taskCreated != pdPASS) {
    delete taskParams;
    isBusArrivalFetchInProgress = false;
    Serial.println("Failed to create bus arrival query task");
    return false;
  }

  return true;
}

BusArrivalCacheMutator busArrivalQuery;
