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
  unique_ptr<BusArrivalApiResponse>* cachedBusArrivalPtr;
  int32_t* lastBusArrivalApiTimestampPtr;
  bool* isBusArrivalFetchInProgressPtr;
  LtaDatamallClient* client;
};

struct WeatherQueryDto {
  NeaWeatherClient* client;
  bool queryTwoHour;
  bool queryTwentyFourHour;
  unique_ptr<TwoHourForecastResponse>* twoHourResponsePtr;
  unique_ptr<TwentyFourHourForecastResponse>* twentyFourHourResponsePtr;
  int32_t* lastWeatherApiTimestampPtr;
  bool* isWeatherFetchInProgressPtr;
};

void HomescreenState::enter() {
  gfx->fillScreen(BG_DARK);
  LayoutRenderer::renderHeader();
  LayoutRenderer::renderFooter();
  gfx->flush();

  lastBusArrivalApiTimestamp = -1;
  lastBusArrivalPaintTimestamp = -1;
  lastDateTimePaintTimestamp = -1;
  lastWeatherApiTimestamp = -1;
  lastWeatherDisplayToggleTimestamp = -1;
  currentWeatherMode = WeatherDisplayMode::TwoHour;

  // Reset pagination
  currentPage = 0;
  totalPages = 1;
  touchStartX = -1;
  touchStartY = -1;
  isSwipeInProgress = false;

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
  if (timeNow - lastBusArrivalApiTimestamp > 60 &&
      !isBusArrivalFetchInProgress) {
    updateBusArrivalCache();
    paintBusArrivalCache();
  }

  // Repaint bus display every 30 seconds
  if (timeNow - lastBusArrivalPaintTimestamp > 30) {
    paintBusArrivalCache();
  }

  // Refresh weather data every 5 minutes
  if (timeNow - lastWeatherApiTimestamp > WEATHER_REFRESH_INTERVAL_SECS &&
      !isWeatherFetchInProgress) {
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
  *dtoPtr->cachedBusArrivalPtr =
      dtoPtr->client->getBusArrival(dtoPtr->busStopCode);
  *dtoPtr->lastBusArrivalApiTimestampPtr = TimeUtil::getUnixTime();
  *dtoPtr->isBusArrivalFetchInProgressPtr = false;

  delete dtoPtr;
  vTaskDelete(NULL);
}

void HomescreenState::queryWeatherWorker(void* pvParameters) {
  WeatherQueryDto* dtoPtr = (WeatherQueryDto*)(pvParameters);

  if (dtoPtr->queryTwoHour) {
    *dtoPtr->twoHourResponsePtr = dtoPtr->client->getTwoHourForecast();
  }

  if (dtoPtr->queryTwentyFourHour) {
    *dtoPtr->twentyFourHourResponsePtr =
        dtoPtr->client->getTwentyFourHourForecast();
  }
  *dtoPtr->lastWeatherApiTimestampPtr = TimeUtil::getUnixTime();
  *dtoPtr->isWeatherFetchInProgressPtr = false;

  delete dtoPtr;
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

  isBusArrivalFetchInProgress = true;
  auto taskParams = new QueryBusArrivalWorkerDto{
      busStopCode, &cachedBusArrival, &lastBusArrivalApiTimestamp,
      &isBusArrivalFetchInProgress, &ltaDatamallClient};
  TaskHandle_t taskHandle = NULL;
  BaseType_t taskCreated = xTaskCreate(queryBusArrivalWorker, "homescreen-bus",
                                       8192, taskParams, 1, &taskHandle);

  if (taskCreated != pdPASS) {
    delete taskParams;
    isBusArrivalFetchInProgress = false;
    Serial.println("Failed to create bus arrival query task");
    busArrivalRenderer.renderError("Task create failed");
  }
}

void HomescreenState::updateWeatherCache() {
  auto taskParams = new WeatherQueryDto{&neaWeatherClient,
                                        true,
                                        true,
                                        &cachedTwoHourForecast,
                                        &cachedTwentyFourHourForecast,
                                        &lastWeatherApiTimestamp,
                                        &isWeatherFetchInProgress};
  isWeatherFetchInProgress = true;
  TaskHandle_t taskHandle = NULL;
  BaseType_t taskCreated = xTaskCreate(queryWeatherWorker, "homescreen-weather",
                                       8192, taskParams, 1, &taskHandle);

  if (taskCreated != pdPASS) {
    Serial.println("Failed to create weather query task");
    delete taskParams;
    isWeatherFetchInProgress = false;
    lastWeatherApiTimestamp = TimeUtil::getUnixTime();
  }
}

void HomescreenState::paintBusArrivalCache() {
  busArrivalRenderer.clearContent();
  if (cachedBusArrival == nullptr) {
    // Paint from settings
    busArrivalRenderer.renderBusStopCode(configManager.getBusStopCode());
    lastBusArrivalPaintTimestamp = TimeUtil::getUnixTime();
    if (lastBusArrivalApiTimestamp > 0) {
      busArrivalRenderer.renderError("API request failed");
    }
    return;
  }

  busArrivalRenderer.renderBusStopCode(cachedBusArrival->busStopCode);

  // Get sorted services
  vector<BusArrivalServiceDetail*> sortedServices = getSortedBusServices();

  // Calculate pagination
  int totalServices = sortedServices.size();
  totalPages =
      (totalServices + 5) / 6;  // Ceiling division (6 services per page)
  if (totalPages == 0) totalPages = 1;
  if (currentPage >= totalPages) currentPage = 0;

  // Display services for current page
  int startIdx = currentPage * 6;
  int endIdx = min(startIdx + 6, totalServices);

  for (int i = startIdx; i < endIdx; i++) {
    busArrivalRenderer.renderBusService(i - startIdx, sortedServices[i]);
  }

  // Always render page indicator (shows "Pg 1/x" for multi-page, bars for
  // single page)
  busArrivalRenderer.renderPageIndicator(currentPage + 1, totalPages);

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

vector<BusArrivalServiceDetail*> HomescreenState::getSortedBusServices() {
  vector<BusArrivalServiceDetail*> result;

  if (cachedBusArrival == nullptr || cachedBusArrival->services.empty()) {
    return result;
  }
  result.reserve(cachedBusArrival->services.size());

  // Get service order configuration (comma-separated string)
  BusServiceOrderConfig configOrdering = configManager.getBusServiceOrder();

  if (configOrdering.size == 0 || configOrdering.services == nullptr) {
    // No custom order - return services in API order
    for (auto& service : cachedBusArrival->services) {
      result.push_back(&service);
    }
    return result;
  }

  // We incur O(n^2) to pick from cached response because we don't want to
  // deal with malloc and hashmap lookups, which can be costly in memory
  // fragmentation
  for (int i = 0; i < configOrdering.size; i++) {
    String serviceNo = configOrdering.services[i];
    for (int j = 0; j < cachedBusArrival->services.size(); j++) {
      BusArrivalServiceDetail* servicePtr = &cachedBusArrival->services[j];
      if (servicePtr->serviceNo == serviceNo) {
        result.push_back(servicePtr);
        break;
      }
    }
  }

  // Incur another O(n^2) to add any remaining services (not in config) at the
  // end
  for (int i = 0; i < cachedBusArrival->services.size(); i++) {
    String serviceNo = cachedBusArrival->services[i].serviceNo;
    bool isFound = false;
    for (int j = 0; j < configOrdering.size; j++) {
      if (serviceNo == configOrdering.services[j]) {
        isFound = true;
        break;
      }
    }

    if (!isFound) {
      result.push_back(&cachedBusArrival->services[i]);
    }
  }

  return result;
}

void HomescreenState::handleTouchStart(int x, int y) {
  touchStartX = x;
  touchStartY = y;
  isSwipeInProgress = true;
}

void HomescreenState::handleTouchEnd(int x, int y) {
  if (!isSwipeInProgress) {
    return;
  }

  isSwipeInProgress = false;

  // Check if this was a swipe
  int deltaX = x - touchStartX;
  int deltaY = abs(y - touchStartY);

  // Must move horizontally more than threshold and not too much vertically
  if (abs(deltaX) >= SWIPE_THRESHOLD && deltaY < SWIPE_MAX_Y_DEVIATION) {
    // Visual feedback - flash the screen briefly
    gfx->fillRect(0, CONTENT_TOP, SCREEN_WIDTH, CONTENT_HEIGHT,
                  0x2965);  // Dim overlay
    gfx->flush();
    delay(50);

    if (deltaX > 0) {
      // Swipe right - previous page
      if (currentPage > 0) {
        currentPage--;
        paintBusArrivalCache();
      }
    } else {
      // Swipe left - next page
      if (currentPage < totalPages - 1) {
        currentPage++;
        paintBusArrivalCache();
      }
    }
  }

  touchStartX = -1;
  touchStartY = -1;
}