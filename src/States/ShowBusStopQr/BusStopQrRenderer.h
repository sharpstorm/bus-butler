#pragma once

#include "../../NeaWeatherApi/NeaWeatherApiTypes.h"
#include "../../Screen/ScreenReference.h"
#include "Arduino.h"

class BusStopQrRenderer {
 public:
  static void renderLayout(const String& busStopCode);
  static void renderCountdown(const int secondsLeft);
  static void clearScreen();

  static constexpr int BACK_BUTTON_X = 300;
  static constexpr int BACK_BUTTON_Y = 190;
  static constexpr int BUTTON_HEIGHT = 36;
  static constexpr int BACK_BUTTON_WIDTH = 60;

  static constexpr int COUNTDOWN_X = 300;
  static constexpr int COUNTDOWN_Y = 40;

 private:
  static void renderHeader();
};
