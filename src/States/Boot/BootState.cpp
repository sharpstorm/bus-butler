#include "BootState.h"

#include "../../Config/ConfigManager.h"
#include "../../Util/Time.h"
#include "../../WebConfigServer/WebConfigServer.h"
#include "../../Wifi/WifiConfigManager.h"
#include "../../Wifi/WifiManager.h"
#include "../AppStateManager.h"

void BootState::enter() {
  if (!isSetupComplete()) {
    appStateManager.changeState(AppState::Setup);
    return;
  }

  renderer.renderInitial();
  wifiManager.connectToWifi();
  renderer.renderWifiConnected();
  TimeUtil::syncer.syncNtpTime();

  // Temporary
  mdnsClient.start();
  webConfigServer.start();

  appStateManager.changeState(AppState::Homescreen);
}

void BootState::loop() {}

void BootState::exit() {}

void BootState::handleTouch(int x, int y) {}

bool BootState::isSetupComplete() {
  bool isWifiConfigured = wifiConfigManager.getConfig().isConfigured;
  bool isDatamallConfigured = configManager.getLtaDatamallApiKey().length() > 0;

  return isWifiConfigured && isDatamallConfigured;
}
