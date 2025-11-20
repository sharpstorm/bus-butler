#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "touchscreen/TouchScreen.h"

#define LCD_CS 45
#define LCD_SCK 47
#define LCD_D0 21
#define LCD_D1 48
#define LCD_D2 40
#define LCD_D3 39
#define LCD_BL_PIN 1

#define LCD_RST GFX_NOT_DEFINED
#define LCD_ROTATION 0
#define LCD_IS_IPS true

#define LCD_WIDTH 480
#define LCD_HEIGHT 272

Arduino_DataBus* bus =
    new Arduino_ESP32QSPI(LCD_CS, LCD_SCK, LCD_D0, LCD_D1, LCD_D2, LCD_D3);
Arduino_GFX* g = new Arduino_NV3041A(bus, LCD_RST, LCD_ROTATION, LCD_IS_IPS);
Arduino_GFX* gfx = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, g);

void setup() {
  setCpuFrequencyMhz(160);
  gfx->begin();
  gfx->fillScreen(BLACK);
  pinMode(LCD_BL_PIN, OUTPUT);
  digitalWrite(LCD_BL_PIN, HIGH);

  gfx->fillCircle(100, 100, 20, RED);
  gfx->flush();

  touchInit();
}

bool isScreenClear = false;

void loop() {
  // put your main code here, to run repeatedly:
  touchLoop();
  if (hasTouch) {
    gfx->fillRect(touchPoint.x - 40, touchPoint.y - 40, 80, 80, BLACK);
    gfx->fillCircle(touchPoint.x, touchPoint.y, 20, RED);
    gfx->flush();
    isScreenClear = false;
  } else if (!isScreenClear) {
    gfx->fillScreen(BLACK);
    gfx->flush();
    isScreenClear = true;
  }
}
