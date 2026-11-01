#include <Arduino.h>
#include <nvs_flash.h>

#include "Display/DisplayPower.h"
#include "Input/Gesture.h"
#include "Notify/NotifyUi.h"
#include "Screen/Colors.h"
#include "Screen/Screen.h"
#include "States/AppStateManager.h"
#include "States/AppStateStore.h"
#include "States/IAppState.h"
#include "TouchScreen/TouchScreen.h"

static GestureTracker gestureTracker;
static bool isPressed = false;
// Set when the press only woke an idle-dimmed screen.
static bool swallowPress = false;
// Set when the press started on a toast: the whole gesture is the toast's.
static bool toastPress = false;

static void handleTouch(IAppState* state) {
  updateTouch();
  uint32_t now = millis();

  if (hasTouch) {
    int16_t x = touchPoint.x, y = touchPoint.y;
    if (!isPressed) {
      isPressed = true;
      swallowPress = displayPower.onTouch();
      gestureTracker.down(x, y, now);
      toastPress = !swallowPress && notifyUi.toastContains(x, y);
      if (!swallowPress && !toastPress) state->onTouchDown(x, y);
    } else if (x != gestureTracker.lastX() || y != gestureTracker.lastY()) {
      gestureTracker.move(x, y);
      displayPower.onTouch();
      if (!swallowPress && !toastPress) state->onTouchMove(x, y);
    }
    return;
  }

  if (isPressed) {
    isPressed = false;
    if (swallowPress) return;
    if (toastPress) {
      // Tap it or flick it away in any direction
      toastPress = false;
      Gesture gesture;
      if (gestureTracker.release(now, gesture)) notifyUi.dismissToastByUser();
      return;
    }
    state->onTouchUp(gestureTracker.lastX(), gestureTracker.lastY());
    Gesture gesture;
    if (gestureTracker.release(now, gesture)) {
      appStateManager.dispatchGesture(gesture);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setCpuFrequencyMhz(160);
  screen.init();
  gfx->setTextWrap(false);
  displayPower.begin();
  touchInit();
  appStateManager.changeState(AppState::Boot);
}

void loop() {
  IAppState* activeState = appStateManager.getActiveState();
  if (activeState == nullptr) {
    appStateManager.changeState(AppState::Boot);
    return;
  }

  handleTouch(activeState);

  // The touch may have changed state
  activeState = appStateManager.getActiveState();
  if (activeState != nullptr) {
    activeState->loop();
  }

  // Pushed notifications: opens / closes the overlay, shows toasts
  notifyUi.loop();

  displayPower.loop();

  // Switch to / from the warm night palette. Overlays redraw from whatever
  // page they return to, so wait until they're closed.
  AppState current = appStateManager.getActiveStateId();
  bool overlay = AppStateManager::isOverlay(current);
  bool drawsPalette = appStateManager.pageIndex(current) >= 0;
  if (!overlay && drawsPalette &&
      displayPower.wantsNightPalette() != isNightPalette()) {
    setNightPalette(displayPower.wantsNightPalette());
    appStateManager.redraw();
  }

  // Self-Reset Routine after 30 days
  if (millis() > 2592000000l) {
    ESP.restart();
  }

  delay(5);
}
