#include "HomescreenState.h"

#include "../../Screen/Fonts/BusTypes.h"
#include "../../Screen/Fonts/RobotoMonoDigits30.h"
#include "../../Screen/Fonts/RobotoMonoDigits40.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"

struct QueryBusArrivalWorkerDto {
  String busStopCode;
  unique_ptr<BusArrivalApiResponse> apiResp;
  LtaDatamallClient* client;
  volatile bool isCompleted;
};

using namespace std;

void HomescreenState::enter() {
  gfx->fillScreen(BLACK);
  lastBusArrivalApiTimestamp = -1;
  lastBusArrivalPaintTimestamp = -1;

  if (ltaDatamallClient.isConfigured()) {
    return;
  }

  ltaDatamallClient.reconfigure();
  if (!ltaDatamallClient.isConfigured()) {
    lastBusArrivalApiTimestamp = 2147483647;
    lastBusArrivalPaintTimestamp = 2147483647;
    gfx->setFont(NULL);
    ScreenRenderer::drawCenterText("LTA Datamall API not configured",
                                   SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  }
}

void HomescreenState::exit() {}

void HomescreenState::loop() {
  long timeNow = TimeUtil::getUnixTime();

  if (timeNow - lastBusArrivalApiTimestamp > 60) {
    updateBusArrivalCache();
    paintBusArrivalCache();
  }

  if (timeNow - lastBusArrivalPaintTimestamp > 30) {
    paintBusArrivalCache();
  }
}

void HomescreenState::handleTouch(int x, int y) {}

void HomescreenState::queryBusArrivalWorker(void* pvParameters) {
  QueryBusArrivalWorkerDto* dtoPtr = (QueryBusArrivalWorkerDto*)(pvParameters);
  dtoPtr->apiResp = dtoPtr->client->getBusArrival(dtoPtr->busStopCode);
  dtoPtr->isCompleted = true;
  vTaskDelete(NULL);
}

void HomescreenState::updateBusArrivalCache() {
  TaskHandle_t taskRef = NULL;
  QueryBusArrivalWorkerDto taskResult{"21361", nullptr, &ltaDatamallClient,
                                      false};
  xTaskCreate(queryBusArrivalWorker, "homescreen-bus", 8000, &taskResult, 1,
              &taskRef);

  while (!taskResult.isCompleted) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  cachedBusArrival = move(taskResult.apiResp);
  lastBusArrivalApiTimestamp = TimeUtil::getUnixTime();
}

void HomescreenState::paintBusArrivalCache() {
  if (cachedBusArrival == nullptr) {
    lastBusArrivalPaintTimestamp = TimeUtil::getUnixTime();
    return;
  }

  busArrivalRenderer.renderToolbar(cachedBusArrival->busStopCode);
  for (int i = 0; i < min(7, (int)cachedBusArrival->services.size()); i++) {
    busArrivalRenderer.renderPosition(i, &cachedBusArrival->services[i]);
  }
  busArrivalRenderer.finalize();
  lastBusArrivalPaintTimestamp = TimeUtil::getUnixTime();
}