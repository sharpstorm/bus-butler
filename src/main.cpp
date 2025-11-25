#include <Arduino.h>

#include "Screen/Screen.h"
#include "States/AppStateManager.h"
#include "States/AppStateStore.h"
#include "States/IAppState.h"
#include "States/Setup/SetupState.h"
#include "TouchScreen/TouchScreen.h"

// Initial state
bool isPressed = false;
AppStateManager stateManager;

void setup() {
  setCpuFrequencyMhz(160);
  gfx->begin();
  gfx->fillScreen(BLACK);
  pinMode(LCD_BL_PIN, OUTPUT);
  digitalWrite(LCD_BL_PIN, HIGH);

  touchInit();

  changeAppState(AppState::Setup);
}

void loop() {
  IAppState* activeState = stateManager.getActiveState();
  // Handle touch
  updateTouch();
  if (!isPressed && hasTouch) {
    isPressed = true;
  } else if (isPressed && !hasTouch) {
    activeState->handleTouch(touchPoint.x, touchPoint.y);
    isPressed = false;
  }

  activeState->loop();
}
