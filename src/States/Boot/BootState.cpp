#include "BootState.h"

#include <HTTPClient.h>
#include <WiFi.h>

#include "../../Config/ConfigManager.h"
#include "../../Data/DataStore.h"
#include "../../Util/Time.h"
#include "../../WebConfigServer/WebConfigServer.h"
#include "../../Wifi/WifiConfigManager.h"
#include "../../Wifi/WifiManager.h"
#include "../AppStateManager.h"

#define MAX_WIFI_ATTEMPTS 3

void BootState::enter() {
  if (!isSetupComplete()) {
    appStateManager.changeState(AppState::Setup);
    return;
  }

  renderer.renderInitial();

  bool wifiConnected = false;
  for (int attempt = 1; attempt <= MAX_WIFI_ATTEMPTS; attempt++) {
    renderer.renderWifiConnecting(attempt, MAX_WIFI_ATTEMPTS);
    wifiConnected = wifiManager.connectToWifi();

    if (wifiConnected) {
      break;
    }

    if (attempt < MAX_WIFI_ATTEMPTS) {
      delay(1000);
    }
  }

  if (!wifiConnected) {
    renderer.renderWifiFailed();
    delay(2000);
    appStateManager.changeState(AppState::Setup);
    return;
  }

  renderer.renderWifiConnected();
  renderer.renderWaitInternet();
  while (!isInternetConnected()) {
    delay(1000);
  }
  renderer.renderInternetConnected();

  renderer.renderSyncingTime();
  while (!TimeUtil::syncer.syncNtpTime()) {
    delay(1000);
  }
  renderer.renderStarting();

  mdnsClient.start();
  webConfigServer.start();
  dataStore.begin();

  appStateManager.changeState(AppState::Home);
}

void BootState::loop() {}

void BootState::exit() {}

bool BootState::isSetupComplete() {
  bool isWifiConfigured = wifiConfigManager.getConfig().isConfigured;
  bool isDatamallConfigured = configManager.getLtaDatamallApiKey().length() > 0;

  return isWifiConfigured && isDatamallConfigured;
}

bool BootState::isInternetConnected() {
  HTTPClient http;
  const char* url = "http://google.com/generate_204";

  http.begin(url);
  http.setTimeout(5000);
  const int httpCode = http.GET();  // Send the request

  if (httpCode > 0 && httpCode == HTTP_CODE_NO_CONTENT) {
    WiFiClient* stream = http.getStreamPtr();
    while (stream->available()) {
      stream->read();
    }
    http.end();
    return true;
  }

  http.end();  // Free resources
  return false;
}
