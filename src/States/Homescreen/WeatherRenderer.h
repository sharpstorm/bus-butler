#pragma once

#include "../../NeaWeatherApi/NeaWeatherApiTypes.h"
#include "../../Screen/ScreenReference.h"
#include "Arduino.h"

class WeatherRenderer {
 private:
  void drawWeatherIcon(int16_t x, int16_t y, WeatherCondition condition,
                       uint16_t color, float scale);
  uint16_t getWeatherIconColor(WeatherCondition condition);

 public:
  void renderWeatherIndicator(WeatherCondition condition, const String& label,
                              bool isTwoHour, const String& temp,
                              const String& timePeriod);
  void renderWeatherError(const char* errorText);
  void finalize();
};
