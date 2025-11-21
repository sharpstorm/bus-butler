#pragma once

#include "Screen.h"

class ScreenRenderer {
 public:
  static void drawCenterText(const char* text, int centerX, int centerY);
  static void drawText(const char* text, int x, int y);
};
