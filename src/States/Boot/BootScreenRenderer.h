#pragma once

class BootScreenRenderer {
 public:
  BootScreenRenderer();
  void renderInitial();

  // Connect Wifi
  void renderWifiConnecting(int attempt, int maxAttempts);
  void renderWifiConnected();
  void renderWifiFailed();

  void renderWaitInternet();
  void renderInternetConnected();

  void renderSyncingTime();
  void renderStarting();
};
