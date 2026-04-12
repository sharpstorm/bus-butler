#include "ShowBusStopQrState.h"

#include "../../Config/ConfigManager.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"
#include "../AppStateManager.h"
#include "../AppStateStore.h"
#include "./BusStopQrRenderer.h"

#define COUNTDOWN_TIME_SECS 10

void ShowBusStopQrState::enter() {
  // Render screen
  String busStopCode = configManager.getBusStopCode();
  BusStopQrRenderer::renderLayout(busStopCode);
  entryTimestamp = TimeUtil::getUnixTime();
}

void ShowBusStopQrState::exit() {}

void ShowBusStopQrState::loop() {
  int secondsLeft = TimeUtil::getUnixTime() - entryTimestamp;
  BusStopQrRenderer::renderCountdown(COUNTDOWN_TIME_SECS - secondsLeft);
  if (secondsLeft > COUNTDOWN_TIME_SECS) {
    appStateManager.changeState(AppState::Homescreen);
  }
}

void ShowBusStopQrState::handleTouch(int x, int y) {
  if (isInHitbox(x, y, BusStopQrRenderer::BACK_BUTTON_X,
                 BusStopQrRenderer::BACK_BUTTON_Y,
                 BusStopQrRenderer::BACK_BUTTON_WIDTH,
                 BusStopQrRenderer::BUTTON_HEIGHT)) {
    appStateManager.changeState(AppState::Homescreen);
  }
}
