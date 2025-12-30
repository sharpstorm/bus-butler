#include "HomescreenState.h"

#include "../../Config/ConfigManager.h"
#include "../../Screen/Fonts/BusTypes.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"

#define TEST_BUS_STOP_CODE "22361"

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

  if (timeNow - lastDateTimePaintTimestamp > 5) {
    lastDateTimePaintTimestamp = timeNow;
    busArrivalRenderer.renderDateTime();
    busArrivalRenderer.finalize();
  }
}

void HomescreenState::handleTouch(int x, int y) {
  // Refresh button
  if (isInHitbox(x, y, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 44, 20, 20)) {
    // Force refresh
    lastBusArrivalApiTimestamp = 0;

    // Settings Button
  } else if (isInHitbox(x, y, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 44, 20, 20)) {
    // TODO: Settings action
  }
}

void HomescreenState::queryBusArrivalWorker(void* pvParameters) {
  QueryBusArrivalWorkerDto* dtoPtr = (QueryBusArrivalWorkerDto*)(pvParameters);
  dtoPtr->apiResp = dtoPtr->client->getBusArrival(dtoPtr->busStopCode);
  dtoPtr->isCompleted = true;
  vTaskDelete(NULL);
}

void HomescreenState::updateBusArrivalCache() {
  TaskHandle_t taskRef = NULL;
  String busStopCode = configManager.getBusStopCode();
  if (busStopCode.length() == 0) {
    busStopCode = TEST_BUS_STOP_CODE;
  }
  if (cachedBusArrival != nullptr &&
      cachedBusArrival->busStopCode != busStopCode) {
    // Clear screen
    gfx->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 60, BLACK);
  }

  QueryBusArrivalWorkerDto taskResult{busStopCode, nullptr, &ltaDatamallClient,
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
  for (int i = 0; i < min(6, (int)cachedBusArrival->services.size()); i++) {
    busArrivalRenderer.renderPosition(i, &cachedBusArrival->services[i]);
  }
  busArrivalRenderer.finalize();
  lastBusArrivalPaintTimestamp = TimeUtil::getUnixTime();
}