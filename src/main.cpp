#include <Arduino.h>

#include "Screen/Screen.h"
#include "States/Setup/SetupState.h"
#include "TouchScreen/TouchScreen.h"

SetupState setupState;

void setup() {
  setCpuFrequencyMhz(160);
  gfx->begin();
  gfx->fillScreen(BLACK);
  pinMode(LCD_BL_PIN, OUTPUT);
  digitalWrite(LCD_BL_PIN, HIGH);

  touchInit();
  setupState.enter();
}

void loop() {
  // put your main code here, to run repeatedly:
  touchLoop();
}
