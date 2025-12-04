#pragma once

#include "Screen.h"

class ScreenRenderer {
 public:
  static void drawCenterText(const char* text, int centerX, int centerY);
  static void drawCenterText(const String& text, int centerX, int centerY);
  static void drawText(const char* text, int x, int y);
  static void drawText(const String& text, int x, int y);
  static void drawRightAlignText(const char* text, int x, int y);
  static void drawRightAlignText(const String& text, int x, int y);
};
