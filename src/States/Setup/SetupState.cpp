#include "SetupState.h"

#include "../../WebConfigServer/WebConfigServer.h"
#include "../../Wifi/WifiManager.h"

void SetupState::enter() {
  setupRenderer.render();
  wifiManager.startWifiHotspot();
  webConfigServer.start();
  mdnsClient.start();
}

void SetupState::loop() {}

void SetupState::exit() {
  wifiManager.stopWifiHotspot();
  mdnsClient.stop();
  webConfigServer.stop();
}

void SetupState::handleTouch(int x, int y) {}
