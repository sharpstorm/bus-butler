#pragma once

#include <Wire.h>

#include "GT911TouchScreen.h"

#define TOUCH_MODULE_ADDR GT911_SLAVE_ADDRESS1
#define TOUCH_SCL 4
#define TOUCH_SDA 8
#define TOUCH_RST 38
#define TOUCH_INT 3

GT911TouchScreen touchScreen(Wire, TOUCH_SDA, TOUCH_SCL, TOUCH_MODULE_ADDR,
                             TOUCH_RST, TOUCH_INT);

TouchPoint touchPoint(0, 0, 0, 0);
bool hasTouch = 0;

void touchInit() { touchScreen.start(); }

void touchLoop() {
  hasTouch = touchScreen.read();
  if (hasTouch) {
    touchPoint = touchScreen.getPoint(0);
  }
}
