#pragma once

#include "Arduino.h"
#include "WiFi.h"

#define HOTSPOT_SSID_PREFIX "Butler-"

class WifiManager {
 public:
  WifiManager();

  // As station
  bool isWifiConfigured();
  bool connectToWifi();
  bool isConnectedToWifi();
  void disconnectWifi();
  String getConfiguredSsid();
  void reconnectIfNeeded();

  // As AP
  void startWifiHotspot();
  void stopWifiHotspot();
  String getConfiguredHotspotSsid();
  String getConfiguredHotspotPassword();
  IPAddress getHotspotGatewayIp();

 private:
  char hotspotName[16];
  char hotspotPassword[11];
};

extern WifiManager wifiManager;
