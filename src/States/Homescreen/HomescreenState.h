#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <memory>
#include <vector>

#include "../../LtaDatamallClient/BusArrivalApiTypes.h"
#include "../../LtaDatamallClient/LtaDatamallClient.h"
#include "../../NeaWeatherApi/NeaWeatherApiClient.h"
#include "../../NeaWeatherApi/NeaWeatherApiTypes.h"
#include "../../Wifi/MDnsClient.h"
#include "../IAppState.h"
#include "BusArrivalRenderer.h"
#include "WeatherRenderer.h"

using namespace std;

// Weather display mode for cycling
enum class WeatherDisplayMode { TwoHour, Morning, Afternoon, Night, Midnight };

class HomescreenState : public IAppState {
 private:
  int32_t lastDateTimePaintTimestamp = -1;

  // Bus arrival state
  int32_t lastBusArrivalApiTimestamp = -1;
  int32_t lastBusArrivalPaintTimestamp = -1;
  bool isBusArrivalFetchInProgress = false;
  unique_ptr<BusArrivalApiResponse> cachedBusArrival = nullptr;
  BusArrivalRenderer busArrivalRenderer;
  LtaDatamallClient ltaDatamallClient;

  // Pagination state
  uint8_t currentPage = 0;
  uint8_t totalPages = 1;

  // Swipe detection state
  int touchStartX = -1;
  int touchStartY = -1;
  bool isSwipeInProgress = false;
  static constexpr int SWIPE_THRESHOLD = 80;        // Minimum pixels for swipe
  static constexpr int SWIPE_MAX_Y_DEVIATION = 50;  // Max vertical movement

  // Weather state
  int32_t lastWeatherApiTimestamp = -1;
  int32_t lastWeatherDisplayToggleTimestamp = -1;
  bool isWeatherFetchInProgress = false;
  unique_ptr<TwoHourForecastResponse> cachedTwoHourForecast = nullptr;
  unique_ptr<TwentyFourHourForecastResponse> cachedTwentyFourHourForecast =
      nullptr;
  NeaWeatherClient neaWeatherClient;
  WeatherDisplayMode currentWeatherMode = WeatherDisplayMode::TwoHour;
  WeatherRenderer weatherRenderer;

  // Worker functions
  static void queryBusArrivalWorker(void* pvParameters);
  static void queryWeatherWorker(void* pvParameters);

  // Update functions
  void updateBusArrivalCache();
  void updateWeatherCache();
  void paintBusArrivalCache();
  void paintWeatherIndicator();

  // Helper functions
  vector<BusArrivalServiceDetail*> getSortedBusServices();
  void handleTouchStart(int x, int y);
  void handleTouchEnd(int x, int y);

  static constexpr TickType_t API_QUERY_TIMEOUT_TICKS = pdMS_TO_TICKS(15000);

  // Weather refresh interval: 5 minutes
  static constexpr int32_t WEATHER_REFRESH_INTERVAL_SECS = 300;
  // Weather display toggle interval: 15 seconds
  static constexpr int32_t WEATHER_TOGGLE_INTERVAL_SECS = 15;

 public:
  void enter();
  void loop();
  void exit();

  void handleTouch(int x, int y);
};
