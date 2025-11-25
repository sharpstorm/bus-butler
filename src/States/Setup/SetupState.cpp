#include "SetupState.h"

#include "../../Config/ConfigManager.h"
#include "../../States/AppStateStore.h"
#include "../../WebConfigServer/WebConfigServer.h"
#include "../../Wifi/WifiConfigManager.h"
#include "../../Wifi/WifiManager.h"

void SetupState::enter() {
  if (isSetupComplete()) {
    changeAppState(AppState::Homescreen);
    return;
  }
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

bool SetupState::isSetupComplete() {
  bool isWifiConfigured = wifiConfigManager.getConfig().isConfigured;
  bool isDatamallConfigured = configManager.getLtaDatamallApiKey().length() > 0;

  return isWifiConfigured && isDatamallConfigured;
}
