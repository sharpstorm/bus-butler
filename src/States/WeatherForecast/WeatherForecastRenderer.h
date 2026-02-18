#pragma once

#include "../../NeaWeatherApi/NeaWeatherApiTypes.h"
#include "../../Screen/ScreenReference.h"
#include "Arduino.h"

class WeatherForecastRenderer {
 private:
  void drawLargeWeatherIcon(int16_t x, int16_t y, WeatherCondition condition,
                            uint16_t color);
  uint16_t getWeatherIconColor(WeatherCondition condition);
  void drawWeatherDetails(WeatherCondition condition, const String& title,
                          const String& description, const String& temp);

 public:
  void renderTwoHourForecast(const String& town, WeatherCondition condition);
  void renderPeriodForecast(const String& periodName,
                            WeatherCondition condition,
                            const String& description, const String& tempRange);
  void renderError(const char* errorText);
  void renderHeader(const String& title);
  void renderNavigationHint();
  void renderDateTime();
  void clearScreen();
  void finalize();
};
