#pragma once

#include "ScreenReference.h"
#include "ScreenRenderer.h"

class Screen {
 public:
  void init();
  void setBrightness(uint8_t brightness);
  uint8_t getBrightness();

 private:
  uint8_t curBrightness = 100;
};

extern Screen screen;
