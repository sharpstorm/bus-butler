#include "HomescreenState.h"

#include "../../Cache/BusArrivalCacheMutator.h"
#include "../../Cache/NetworkDataCache.h"
#include "../../Cache/WeatherCacheMutator.h"
#include "../../Config/ConfigManager.h"
#include "../../LtaDatamallClient/LtaDatamallClient.h"
#include "../../Screen/Colors.h"
#include "../../Screen/Fonts/BusTypes.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"
#include "../AppStateManager.h"
#include "LayoutRenderer.h"

#define TEST_BUS_STOP_CODE "22361"

using namespace std;

void HomescreenState::enter() {
  gfx->fillScreen(BG_DARK);
  LayoutRenderer::renderHeader();
  LayoutRenderer::renderFooter();
  gfx->flush();

  busArrivalQuery.invalidateCache();
  weatherQuery.invalidateCache();

  lastBusArrivalPaintTimestamp = -1;
  lastDateTimePaintTimestamp = -1;
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
    busArrivalQuery.disable();
    lastBusArrivalPaintTimestamp = INT32_MAX;
    busArrivalRenderer.renderError("API not configured");
  }
}

void HomescreenState::exit() {}

void HomescreenState::loop() {
  long timeNow = TimeUtil::getUnixTime();

  // Refresh bus data every 60 seconds
  if (busArrivalQuery.shouldFetch()) {
    updateBusArrivalCache();
    paintBusArrivalCache();
  }

  // Repaint bus display every 30 seconds
  if (timeNow - lastBusArrivalPaintTimestamp > 30) {
    paintBusArrivalCache();
  }

  // Refresh weather data every 5 minutes
  if (weatherQuery.shouldFetch()) {
    weatherQuery.updateCache();
    paintWeatherIndicator();
    busArrivalRenderer.finalize();
  }

  // Toggle weather display mode every 15 seconds
  if (timeNow - lastWeatherDisplayToggleTimestamp >
      WEATHER_TOGGLE_INTERVAL_SECS) {
    lastWeatherDisplayToggleTimestamp = timeNow;

    // Cycle through all weather display modes
    switch (currentWeatherMode) {
      case WeatherDisplayMode::TwoHour:
        currentWeatherMode = WeatherDisplayMode::Morning;
        break;
      case WeatherDisplayMode::Morning:
        currentWeatherMode = WeatherDisplayMode::Afternoon;
        break;
      case WeatherDisplayMode::Afternoon:
        currentWeatherMode = WeatherDisplayMode::Night;
        break;
      case WeatherDisplayMode::Night:
        currentWeatherMode = WeatherDisplayMode::TwoHour;
        break;
      case WeatherDisplayMode::Midnight:
        currentWeatherMode = WeatherDisplayMode::Midnight;
        break;
    }

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
    busArrivalQuery.invalidateCache();
    weatherQuery.invalidateCache();
  }
  // Settings button
  else if (isInHitbox(x, y, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 50, 32, 32)) {
    appStateManager.changeState(AppState::Settings);
  }
  // Weather area tap - toggle display mode
  else if (isInHitbox(x, y, 150, SCREEN_HEIGHT - FOOTER_HEIGHT, 230,
                      FOOTER_HEIGHT)) {
    // Cycle through all weather display modes
    switch (currentWeatherMode) {
      case WeatherDisplayMode::TwoHour:
        currentWeatherMode = WeatherDisplayMode::Morning;
        break;
      case WeatherDisplayMode::Morning:
        currentWeatherMode = WeatherDisplayMode::Afternoon;
        break;
      case WeatherDisplayMode::Afternoon:
        currentWeatherMode = WeatherDisplayMode::Night;
        break;
      case WeatherDisplayMode::Night:
        currentWeatherMode = WeatherDisplayMode::Midnight;
        break;
      case WeatherDisplayMode::Midnight:
        currentWeatherMode = WeatherDisplayMode::TwoHour;
        break;
    }
    lastWeatherDisplayToggleTimestamp = TimeUtil::getUnixTime();
    paintWeatherIndicator();
    busArrivalRenderer.finalize();
  }
}

void HomescreenState::updateBusArrivalCache() {
  TaskHandle_t taskRef = NULL;
  String busStopCode = configManager.getBusStopCode();
  if (busStopCode.length() == 0) {
    busStopCode = TEST_BUS_STOP_CODE;
  }

  auto cachedBusArrival = networkDataCache.cachedBusArrival.get();
  if (cachedBusArrival != nullptr &&
      cachedBusArrival->busStopCode != busStopCode) {
    busArrivalRenderer.clearContent();
  }

  if (!busArrivalQuery.updateCache(busStopCode)) {
    busArrivalRenderer.clearContent();
    busArrivalRenderer.renderError("Task create failed");
  }
}

void HomescreenState::paintBusArrivalCache() {
  busArrivalRenderer.clearContent();
  auto cachedBusArrival = networkDataCache.cachedBusArrival.get();
  if (cachedBusArrival == nullptr) {
    // Paint from settings
    busArrivalRenderer.renderBusStopCode(configManager.getBusStopCode());
    lastBusArrivalPaintTimestamp = TimeUtil::getUnixTime();
    if (busArrivalQuery.getLastApiCallTimestamp() > 0) {
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

    if (networkDataCache.cachedTwoHourForecast == nullptr) {
      weatherRenderer.renderWeatherError("Loading...");
      return;
    }

    WeatherCondition condition = neaWeatherClient.getForecastForArea(
        networkDataCache.cachedTwoHourForecast.get(), town);

    String label = weatherConditionToShortString(condition);

    weatherRenderer.renderWeatherIndicator(condition, label, true, "", "");

  } else {
    // 24 hour forecast - Morning, Afternoon, or Night
    auto cachedTwentyFourHourForecast =
        networkDataCache.cachedTwentyFourHourForecast.get();
    if (cachedTwentyFourHourForecast == nullptr) {
      weatherRenderer.renderWeatherError("Loading...");
      return;
    }

    String region = configManager.getWeatherRegion();

    // Get temperature range
    String tempRange =
        String(cachedTwentyFourHourForecast->temperature.low) + "-" +
        String(cachedTwentyFourHourForecast->temperature.high) + "C";

    // Select the appropriate period forecast
    PeriodForecast* periodForecast = nullptr;
    String periodLabel = "";

    switch (currentWeatherMode) {
      case WeatherDisplayMode::Morning:
        periodForecast = &cachedTwentyFourHourForecast->morning;
        periodLabel = MORNING_STRING;
        break;
      case WeatherDisplayMode::Afternoon:
        periodForecast = &cachedTwentyFourHourForecast->afternoon;
        periodLabel = AFTERNOON_STRING;
        break;
      case WeatherDisplayMode::Night:
        periodForecast = &cachedTwentyFourHourForecast->night;
        periodLabel = NIGHT_STRING;
        break;
      case WeatherDisplayMode::Midnight:
        periodForecast = &cachedTwentyFourHourForecast->midnight;
        periodLabel = MIDNIGHT_STRING;
        break;
      default:
        break;
    }

    if (periodForecast == nullptr) {
      weatherRenderer.renderWeatherError("Error");
      return;
    }

    // Get forecast for the configured region
    WeatherCondition condition;
    if (region == "west") {
      condition = periodForecast->regionWest;
    } else if (region == "east") {
      condition = periodForecast->regionEast;
    } else if (region == "central") {
      condition = periodForecast->regionCentral;
    } else if (region == "north") {
      condition = periodForecast->regionNorth;
    } else if (region == "south") {
      condition = periodForecast->regionSouth;
    } else {
      condition = periodForecast->regionCentral;  // Default to central
    }

    String conditionLabel = weatherConditionToShortString(condition);

    // Capitalize first letter if needed
    if (periodLabel.length() > 0) {
      periodLabel[0] = toupper(periodLabel[0]);
    }

    weatherRenderer.renderWeatherIndicator(condition, conditionLabel, false,
                                           tempRange, periodLabel);
  }
}

vector<BusArrivalServiceDetail*> HomescreenState::getSortedBusServices() {
  vector<BusArrivalServiceDetail*> result;
  auto cachedBusArrival = networkDataCache.cachedBusArrival.get();

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

  // Check for vertical swipe down (go to weather forecast)
  if (abs(deltaY) >= SWIPE_THRESHOLD && abs(deltaX) < SWIPE_MAX_DEVIATION) {
    if (deltaY > 0) {
      // Swipe down - go to weather forecast page
      appStateManager.changeState(AppState::WeatherForecast);
      return;
    }
  }

  // Must move horizontally more than threshold and not too much vertically
  if (abs(deltaX) >= SWIPE_THRESHOLD && deltaY < SWIPE_MAX_DEVIATION) {
    bool pageChanged = false;

    if (deltaX > 0) {
      // Swipe right - previous page
      if (currentPage > 0) {
        currentPage--;
        pageChanged = true;
      }
    } else {
      // Swipe left - next page
      if (currentPage < totalPages - 1) {
        currentPage++;
        pageChanged = true;
      }
    }

    // Show visual feedback only if page changed
    if (pageChanged) {
      gfx->fillRect(0, CONTENT_TOP, SCREEN_WIDTH, CONTENT_HEIGHT,
                    0x2965);  // Dim overlay
      gfx->flush();
      delay(50);
      paintBusArrivalCache();
    }
  }

  touchStartX = -1;
  touchStartY = -1;
}
