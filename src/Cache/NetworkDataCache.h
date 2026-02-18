#pragma once

#include <memory>

#include "../LtaDatamallClient/BusArrivalApiTypes.h"
#include "../NeaWeatherApi/NeaWeatherApiTypes.h"

using namespace std;

class NetworkDataCache {
 public:
  unique_ptr<TwoHourForecastResponse> cachedTwoHourForecast = nullptr;
  unique_ptr<TwentyFourHourForecastResponse> cachedTwentyFourHourForecast =
      nullptr;
  unique_ptr<BusArrivalApiResponse> cachedBusArrival = nullptr;
};

extern NetworkDataCache networkDataCache;
