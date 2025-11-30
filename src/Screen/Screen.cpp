#include "Screen.h"

#include "../Config/ConfigManager.h"

void Screen::init() {
  pinMode(LCD_BL_PIN, OUTPUT);
  analogWrite(LCD_BL_PIN, 1);
  gfx->begin();
  gfx->fillScreen(BLACK);

  uint8_t brightness = configManager.getBrightness();
  setBrightness(brightness);
}

void Screen::setBrightness(uint8_t brightness) {
  curBrightness = min(brightness, (uint8_t)255);
  analogWrite(LCD_BL_PIN, curBrightness);
}

uint8_t Screen::getBrightness() { return curBrightness; }

Screen screen;
