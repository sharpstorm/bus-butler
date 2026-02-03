#include <Arduino.h>
#include <nvs_flash.h>

#include "Screen/Screen.h"
#include "States/AppStateManager.h"
#include "States/AppStateStore.h"
#include "States/IAppState.h"
#include "States/Setup/SetupState.h"
#include "TouchScreen/TouchScreen.h"

// Initial state
bool isPressed = false;

void setup() {
  Serial.begin(115200);
  setCpuFrequencyMhz(160);
  screen.init();
  touchInit();
  appStateManager.changeState(AppState::Boot);
}

void loop() {
  IAppState* activeState = appStateManager.getActiveState();
  if (activeState == nullptr) {
    appStateManager.changeState(AppState::Boot);
    return;
  }

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
