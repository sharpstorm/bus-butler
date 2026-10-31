#include "SetupState.h"

#include "../../Config/ConfigManager.h"
#include "../../States/AppStateManager.h"
#include "../../WebConfigServer/WebConfigServer.h"
#include "../../Wifi/WifiConfigManager.h"
#include "../../Wifi/WifiManager.h"

void SetupState::enter() {
  if (isSetupComplete()) {
    appStateManager.changeState(AppState::Boot);
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

bool SetupState::isSetupComplete() {
  bool isWifiConfigured = wifiConfigManager.getConfig().isConfigured;
  bool isDatamallConfigured = configManager.getLtaDatamallApiKey().length() > 0;

  return isWifiConfigured && isDatamallConfigured;
}
