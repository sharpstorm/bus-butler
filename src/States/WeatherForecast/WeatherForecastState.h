#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <memory>

#include "../../NeaWeatherApi/NeaWeatherApiClient.h"
#include "../../NeaWeatherApi/NeaWeatherApiTypes.h"
#include "../IAppState.h"
#include "WeatherForecastRenderer.h"

using namespace std;

enum class WeatherForecastStateDisplayMode {
  TwoHour = 0,
  Morning,
  Afternoon,
  Night,
  Midnight
};

class WeatherForecastState : public IAppState {
 private:
  // Swipe detection state
  int touchStartX = -1;
  int touchStartY = -1;
  bool isSwipeInProgress = false;
  static constexpr int SWIPE_THRESHOLD = 80;  // Minimum pixels for swipe
  static constexpr int SWIPE_MAX_DEVIATION =
      50;  // Max horizontal movement for vertical swipe
  static constexpr int WEATHER_TOGGLE_INTERVAL_SECS = 15;
  int32_t lastWeatherDisplayToggleTimestamp = -1;
  int32_t lastDateTimePaintTimestamp = -1;

  // Current view mode
  WeatherForecastStateDisplayMode currentView =
      WeatherForecastStateDisplayMode::TwoHour;

  // Helper methods
  void renderWeatherView();
  void handleTouchStart(int x, int y);
  void handleTouchEnd(int x, int y);

  WeatherForecastRenderer weatherRenderer;

 public:
  void enter();
  void loop();
  void exit();

  void handleTouch(int x, int y);
};
