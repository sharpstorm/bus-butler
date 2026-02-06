#include "HomescreenState.h"

#include "../../Config/ConfigManager.h"
#include "../../Screen/Colors.h"
#include "../../Screen/Fonts/BusTypes.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"
#include "../AppStateManager.h"
#include "LayoutRenderer.h"

#define TEST_BUS_STOP_CODE "22361"

using namespace std;

struct QueryBusArrivalWorkerDto {
  String busStopCode;
  unique_ptr<BusArrivalApiResponse> apiResp;
  LtaDatamallClient* client;
  volatile bool isCompleted;
};

struct WeatherQueryDto {
  NeaWeatherClient* client;
  unique_ptr<TwoHourForecastResponse> twoHourResponse;
  unique_ptr<TwentyFourHourForecastResponse> twentyFourHourResponse;
  bool queryTwoHour;
  bool queryTwentyFourHour;
  volatile bool isCompleted;
};

void HomescreenState::enter() {
  gfx->fillScreen(BG_DARK);

  lastBusArrivalApiTimestamp = -1;
  lastBusArrivalPaintTimestamp = -1;
  lastDateTimePaintTimestamp = -1;
  lastWeatherApiTimestamp = -1;
  lastWeatherDisplayToggleTimestamp = -1;
  currentWeatherMode = WeatherDisplayMode::TwoHour;

  if (ltaDatamallClient.isConfigured()) {
    return;
  }

  ltaDatamallClient.reconfigure();
  if (!ltaDatamallClient.isConfigured()) {
    lastBusArrivalApiTimestamp = INT32_MAX;
    lastBusArrivalPaintTimestamp = INT32_MAX;
    busArrivalRenderer.renderError("API not configured");
  }
}

void HomescreenState::exit() {}

void HomescreenState::loop() {
  long timeNow = TimeUtil::getUnixTime();

  // Refresh bus data every 60 seconds
  if (timeNow - lastBusArrivalApiTimestamp > 60) {
    updateBusArrivalCache();
    paintBusArrivalCache();
  }

  // Repaint bus display every 30 seconds
  if (timeNow - lastBusArrivalPaintTimestamp > 30) {
    paintBusArrivalCache();
  }

  // Refresh weather data every 5 minutes
  if (timeNow - lastWeatherApiTimestamp > WEATHER_REFRESH_INTERVAL_SECS) {
    updateWeatherCache();
    paintWeatherIndicator();
    busArrivalRenderer.finalize();
  }

  // Toggle weather display mode every 15 seconds
  if (timeNow - lastWeatherDisplayToggleTimestamp >
      WEATHER_TOGGLE_INTERVAL_SECS) {
    lastWeatherDisplayToggleTimestamp = timeNow;
    currentWeatherMode = (currentWeatherMode == WeatherDisplayMode::TwoHour)
                             ? WeatherDisplayMode::TwentyFourHour
                             : WeatherDisplayMode::TwoHour;
    paintWeatherIndicator();
    busArrivalRenderer.finalize();
  }

  // Update time display every 5 seconds
  if (timeNow - lastDateTimePaintTimestamp > 5) {
    lastDateTimePaintTimestamp = timeNow;
    LayoutRenderer::renderDateTime();
    busArrivalRenderer.finalize();
  }
}

void HomescreenState::handleTouch(int x, int y) {
  // Refresh button
  if (isInHitbox(x, y, SCREEN_WIDTH - 40, SCREEN_HEIGHT - 50, 32, 32)) {
    lastBusArrivalApiTimestamp = 0;
    lastWeatherApiTimestamp = 0;
  }
  // Settings button
  else if (isInHitbox(x, y, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 50, 32, 32)) {
    appStateManager.changeState(AppState::Settings);
  }
  // Weather area tap - toggle display mode
  else if (isInHitbox(x, y, 150, SCREEN_HEIGHT - FOOTER_HEIGHT, 230,
                      FOOTER_HEIGHT)) {
    currentWeatherMode = (currentWeatherMode == WeatherDisplayMode::TwoHour)
                             ? WeatherDisplayMode::TwentyFourHour
                             : WeatherDisplayMode::TwoHour;
    lastWeatherDisplayToggleTimestamp = TimeUtil::getUnixTime();
    paintWeatherIndicator();
    busArrivalRenderer.finalize();
  }
}

void HomescreenState::queryBusArrivalWorker(void* pvParameters) {
  QueryBusArrivalWorkerDto* dtoPtr = (QueryBusArrivalWorkerDto*)(pvParameters);
  dtoPtr->apiResp = dtoPtr->client->getBusArrival(dtoPtr->busStopCode);
  dtoPtr->isCompleted = true;

  vTaskDelete(NULL);
}

void HomescreenState::queryWeatherWorker(void* pvParameters) {
  WeatherQueryDto* dtoPtr = (WeatherQueryDto*)(pvParameters);

  if (dtoPtr->queryTwoHour) {
    dtoPtr->twoHourResponse = dtoPtr->client->getTwoHourForecast();
  }
  if (dtoPtr->queryTwentyFourHour) {
    dtoPtr->twentyFourHourResponse =
        dtoPtr->client->getTwentyFourHourForecast();
  }
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
    busArrivalRenderer.clearContent();
  }

  QueryBusArrivalWorkerDto taskResult{busStopCode, nullptr, &ltaDatamallClient,
                                      false};
  TaskHandle_t taskHandle = NULL;
  BaseType_t taskCreated = xTaskCreate(queryBusArrivalWorker, "homescreen-bus",
                                       8192, &taskResult, 1, &taskHandle);

  if (taskCreated != pdPASS) {
    Serial.println("Failed to create bus arrival query task");
    busArrivalRenderer.renderError("Task create failed");
    return;
  }

  while (!taskResult.isCompleted) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  cachedBusArrival = move(taskResult.apiResp);
  lastBusArrivalApiTimestamp = TimeUtil::getUnixTime();

  if (cachedBusArrival == nullptr) {
    busArrivalRenderer.renderError("API request failed");
  }
}

void HomescreenState::updateWeatherCache() {
  WeatherQueryDto taskResult{
      &neaWeatherClient, nullptr, nullptr, true, true, false};
  TaskHandle_t taskHandle = NULL;
  BaseType_t taskCreated = xTaskCreate(queryWeatherWorker, "homescreen-weather",
                                       8192, &taskResult, 1, &taskHandle);

  if (taskCreated != pdPASS) {
    Serial.println("Failed to create weather query task");
    lastWeatherApiTimestamp = TimeUtil::getUnixTime();
    return;
  }

  while (!taskResult.isCompleted) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  if (taskResult.twoHourResponse != nullptr) {
    cachedTwoHourForecast = move(taskResult.twoHourResponse);
  }
  if (taskResult.twentyFourHourResponse != nullptr) {
    cachedTwentyFourHourForecast = move(taskResult.twentyFourHourResponse);
  }
  lastWeatherApiTimestamp = TimeUtil::getUnixTime();
}

void HomescreenState::paintBusArrivalCache() {
  if (cachedBusArrival == nullptr) {
    lastBusArrivalPaintTimestamp = TimeUtil::getUnixTime();
    return;
  }

  busArrivalRenderer.clearContent();
  int numServices = min(6, (int)cachedBusArrival->services.size());
  for (int i = 0; i < numServices; i++) {
    busArrivalRenderer.renderBusService(i, &cachedBusArrival->services[i]);
  }

  LayoutRenderer::renderHeader();
  busArrivalRenderer.renderBusStopCode(cachedBusArrival->busStopCode);
  LayoutRenderer::renderFooter();
  LayoutRenderer::renderDateTime();
  paintWeatherIndicator();

  busArrivalRenderer.finalize();
  lastBusArrivalPaintTimestamp = TimeUtil::getUnixTime();
}

void HomescreenState::paintWeatherIndicator() {
  if (currentWeatherMode == WeatherDisplayMode::TwoHour) {
    // Two hour forecast - need town config
    String town = configManager.getWeatherTown();

    if (town.length() == 0) {
      weatherRenderer.renderWeatherError("Set town");
      return;
    }

    if (cachedTwoHourForecast == nullptr) {
      weatherRenderer.renderWeatherError("Loading...");
      return;
    }

    WeatherCondition condition =
        neaWeatherClient.getForecastForArea(cachedTwoHourForecast.get(), town);

    String label = weatherConditionToShortString(condition);

    weatherRenderer.renderWeatherIndicator(condition, label, true, "");

  } else {
    // 24 hour forecast
    if (cachedTwentyFourHourForecast == nullptr) {
      weatherRenderer.renderWeatherError("Loading...");
      return;
    }

    String region = configManager.getWeatherRegion();

    // Get temperature range
    String tempRange =
        String(cachedTwentyFourHourForecast->temperature.low) + "-" +
        String(cachedTwentyFourHourForecast->temperature.high) + "C";

    // Get forecast text for region
    WeatherCondition condition;
    if (region == "west") {
      condition = cachedTwentyFourHourForecast->morning.regionWest;
    } else if (region == "east") {
      condition = cachedTwentyFourHourForecast->morning.regionEast;
    } else if (region == "central") {
      condition = cachedTwentyFourHourForecast->morning.regionCentral;
    } else if (region == "north") {
      condition = cachedTwentyFourHourForecast->morning.regionNorth;
    } else if (region == "south") {
      condition = cachedTwentyFourHourForecast->morning.regionSouth;
    }

    String label = weatherConditionToShortString(condition);
    weatherRenderer.renderWeatherIndicator(condition, label, false, tempRange);
  }
}