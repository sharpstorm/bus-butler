#pragma once

#include <memory>

#include "../../LtaDatamallClient/BusArrivalApiTypes.h"
#include "../../LtaDatamallClient/LtaDatamallClient.h"
#include "../../Wifi/MDnsClient.h"
#include "../IAppState.h"
#include "BusArrivalRenderer.h"

class HomescreenState : public IAppState {
 private:
  int32_t lastBusArrivalApiTimestamp = -1;
  int32_t lastBusArrivalPaintTimestamp = -1;
  unique_ptr<BusArrivalApiResponse> cachedBusArrival = nullptr;
  BusArrivalRenderer busArrivalRenderer;
  LtaDatamallClient ltaDatamallClient;

  static void queryBusArrivalWorker(void* pvParameters);

  void updateBusArrivalCache();
  void paintBusArrivalCache();

 public:
  void enter();
  void loop();
  void exit();

  void handleTouch(int x, int y);
};
