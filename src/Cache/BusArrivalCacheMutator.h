#pragma once

#include <memory>

#include "../LtaDatamallClient/BusArrivalApiTypes.h"
#include "../LtaDatamallClient/LtaDatamallClient.h"

using namespace std;

class BusArrivalCacheMutator {
 private:
  int32_t lastBusArrivalApiTimestamp = -1;
  bool isBusArrivalFetchInProgress = false;

  static void queryBusArrivalWorker(void* pvParameters);

  // Bus arrival refresh interval: 1 min
  static constexpr int32_t BUS_ARRIVAL_REFRESH_INTERVAL_SECS = 60;

 public:
  bool isStale();
  bool shouldFetch();

  void invalidateCache();
  void disable();

  int32_t getLastApiCallTimestamp();

  /* Returns if successful*/
  bool updateCache(String busStopCode);
};

extern BusArrivalCacheMutator busArrivalQuery;
