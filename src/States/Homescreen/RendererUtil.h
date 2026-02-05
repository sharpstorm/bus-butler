#pragma once

#include "../../Screen/ScreenReference.h"
#include "Arduino.h"

class RendererUtil {
 public:
  static void drawCornerBrackets(int16_t x, int16_t y, int16_t w, int16_t h,
                                 uint16_t color, int16_t len);
};
