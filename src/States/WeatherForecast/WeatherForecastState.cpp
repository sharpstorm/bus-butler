#include "WeatherForecastState.h"

#include "../../Cache/NetworkDataCache.h"
#include "../../Cache/WeatherCacheMutator.h"
#include "../../Config/ConfigManager.h"
#include "../../Screen/Colors.h"
#include "../../Screen/Screen.h"
#include "../AppStateManager.h"
#include "../AppStateStore.h"
#include "../Homescreen/HomescreenState.h"

WeatherForecastStateDisplayMode& operator--(
    WeatherForecastStateDisplayMode& mode) {
  // Cast to underlying type, decrement
  int val = static_cast<int>(mode);
  val = val >= 1 ? val - 1 : 4;
  mode = static_cast<WeatherForecastStateDisplayMode>(val);
  return mode;
}

WeatherForecastStateDisplayMode operator--(
    WeatherForecastStateDisplayMode& mode, int) {
  WeatherForecastStateDisplayMode copy = mode;
  --mode;  // Call the pre-decrement operator
  return mode;
}

WeatherForecastStateDisplayMode& operator++(
    WeatherForecastStateDisplayMode& mode) {
  // Cast to underlying type, decrement
  int val = static_cast<int>(mode);
  val = val >= 4 ? 0 : val + 1;
  mode = static_cast<WeatherForecastStateDisplayMode>(val);
  return mode;
}

WeatherForecastStateDisplayMode operator++(
    WeatherForecastStateDisplayMode& mode, int) {
  WeatherForecastStateDisplayMode copy = mode;
  ++mode;  // Call the pre-decrement operator
  return mode;
}

void WeatherForecastState::enter() {
  currentView =
      WeatherForecastStateDisplayMode::TwoHour;  // Start with 2-hour forecast
  lastWeatherDisplayToggleTimestamp = TimeUtil::getUnixTime();
  lastDateTimePaintTimestamp = TimeUtil::getUnixTime();
  touchStartX = -1;
  touchStartY = -1;
  isSwipeInProgress = false;

  renderWeatherView();
}

void WeatherForecastState::exit() {}

void WeatherForecastState::loop() {
  long timeNow = TimeUtil::getUnixTime();

  if (weatherQuery.shouldFetch()) {
    weatherQuery.updateCache();
    renderWeatherView();
  }

  // Toggle weather display mode every 15 seconds
  if (timeNow - lastWeatherDisplayToggleTimestamp >
      WEATHER_TOGGLE_INTERVAL_SECS) {
    lastWeatherDisplayToggleTimestamp = timeNow;

    // Cycle through all weather display modes
    currentView++;
    renderWeatherView();
  }

  if (timeNow - lastDateTimePaintTimestamp > 5) {
    lastDateTimePaintTimestamp = timeNow;
    weatherRenderer.renderDateTime();
    weatherRenderer.finalize();
  }
}

void WeatherForecastState::handleTouch(int x, int y) {
  // Touch handling is done via handleTouchStart and handleTouchEnd
}

void WeatherForecastState::handleTouchStart(int x, int y) {
  touchStartX = x;
  touchStartY = y;
  isSwipeInProgress = true;
}

void WeatherForecastState::handleTouchEnd(int x, int y) {
  if (!isSwipeInProgress) {
    return;
  }

  isSwipeInProgress = false;

  int deltaX = x - touchStartX;
  int deltaY = y - touchStartY;

  // Check for vertical swipe (up to go back to homescreen)
  if (abs(deltaY) >= SWIPE_THRESHOLD && abs(deltaX) < SWIPE_MAX_DEVIATION) {
    if (deltaY > 0) {
      // Swipe down - do nothing (already on weather page)
    } else {
      // Swipe up - go back to homescreen
      appStateManager.changeState(AppState::Homescreen);
    }
  }
  // Check for horizontal swipe (cycle through forecasts)
  else if (abs(deltaX) >= SWIPE_THRESHOLD &&
           abs(deltaY) < SWIPE_MAX_DEVIATION) {
    if (deltaX > 0) {
      // Swipe right - previous view
      currentView--;
    } else {
      // Swipe left - next view
      currentView++;
    }
    renderWeatherView();
  }

  touchStartX = -1;
  touchStartY = -1;
}

void WeatherForecastState::renderWeatherView() {
  String town = configManager.getWeatherTown();
  String region = configManager.getWeatherRegion();

  if (currentView == WeatherForecastStateDisplayMode::TwoHour) {
    // 2-Hour Forecast
    if (town.length() == 0) {
      weatherRenderer.renderError("Configure town");
      weatherRenderer.finalize();
      return;
    }

    if (networkDataCache.cachedTwoHourForecast == nullptr) {
      weatherRenderer.renderError("Loading...");
      weatherRenderer.finalize();
      return;
    }

    WeatherCondition condition = neaWeatherClient.getForecastForArea(
        networkDataCache.cachedTwoHourForecast.get(), town);
    weatherRenderer.renderTwoHourForecast(town, condition);
    weatherRenderer.finalize();

  } else {
    // 24-Hour Forecast (Morning, Afternoon, Night)
    if (networkDataCache.cachedTwentyFourHourForecast == nullptr) {
      weatherRenderer.renderError("Loading...");
      weatherRenderer.finalize();
      return;
    }

    PeriodForecast* periodForecast = nullptr;
    String periodName = "";

    switch (currentView) {
      case WeatherForecastStateDisplayMode::Morning:  // Morning
        periodForecast =
            &networkDataCache.cachedTwentyFourHourForecast->morning;
        periodName = "MORNING";
        break;
      case WeatherForecastStateDisplayMode::Afternoon:  // Afternoon
        periodForecast =
            &networkDataCache.cachedTwentyFourHourForecast->afternoon;
        periodName = "AFTERNOON";
        break;
      case WeatherForecastStateDisplayMode::Night:  // Night
        periodForecast = &networkDataCache.cachedTwentyFourHourForecast->night;
        periodName = "NIGHT";
        break;
      case WeatherForecastStateDisplayMode::Midnight:  // Midnight
        periodForecast =
            &networkDataCache.cachedTwentyFourHourForecast->midnight;
        periodName = "MIDNIGHT";
        break;
    }

    if (periodForecast == nullptr) {
      weatherRenderer.renderError("Error");
      weatherRenderer.finalize();
      return;
    }

    // Get condition for configured region
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
      condition = periodForecast->regionCentral;  // Default
    }

    String tempRange =
        String(networkDataCache.cachedTwentyFourHourForecast->temperature.low) +
        "-" +
        String(
            networkDataCache.cachedTwentyFourHourForecast->temperature.high) +
        "C";

    String description = weatherConditionToString(condition);

    weatherRenderer.renderPeriodForecast(periodName, condition, description,
                                         tempRange);
    weatherRenderer.finalize();
  }
}
