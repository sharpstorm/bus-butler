#pragma once

#include "../../Screen/ScreenReference.h"
#include "Arduino.h"

class LayoutRenderer {
 public:
  static void renderFooter();
  static void renderDateTime();
  static void renderHeader();
};
