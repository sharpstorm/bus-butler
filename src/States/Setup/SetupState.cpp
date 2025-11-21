#include "SetupState.h"

#include "../../Wifi/WifiManager.h"

void SetupState::enter() {
  wifiManager.startWifiHotspot();
  setupRenderer.render();
  mdnsClient.start();
}

void SetupState::loop() {}

void SetupState::exit() {
  wifiManager.stopWifiHotspot();
  mdnsClient.stop();
}

void SetupState::handleTouch(int x, int y) {}