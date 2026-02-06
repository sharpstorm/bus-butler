#include "WifiManager.h"

#include "../Util/Base32.h"
#include "../Util/HardwareId.h"
#include "WifiConfigManager.h"

// Reliable public DNS servers
static const IPAddress DNS_PRIMARY(1, 1, 1, 1);    // CloudFlare
static const IPAddress DNS_SECONDARY(8, 8, 8, 8);  // Google

WifiManager::WifiManager() {
  // Compute hotspot SSID
  // Secret is 32 bytes
  unique_ptr<byte[]> hashSecret = HardwareId::getHardwareIdHash();
  Base32Result hashResult = Base32::toBase32(hashSecret.get(), 256);

  char basePrefix[] = HOTSPOT_SSID_PREFIX;
  strcpy(hotspotName, basePrefix);
  memcpy(&(hotspotName[7]), hashResult.result.get(), 8);
  hotspotName[15] = 0;

  memcpy(hotspotPassword, hashResult.result.get() + 8, 10);
  hotspotPassword[10] = 0;
}

bool WifiManager::isWifiConfigured() {
  WifiConfig config = wifiConfigManager.getConfig();
  return config.isConfigured;
}

String WifiManager::getConfiguredSsid() {
  WifiConfig config = wifiConfigManager.getConfig();
  if (!config.isConfigured) {
    return "Unconfigured WiFi";
  }
  return config.ssid;
}

bool WifiManager::connectToWifi() {
  uint64_t timeSinceConnection = millis();
  WifiConfig config = wifiConfigManager.getConfig();
  if (!config.isConfigured) {
    return false;
  }

  WiFi.begin(config.ssid, config.passphrase);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - timeSinceConnection > 10000) {
      return false;
    }
  }

  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), DNS_PRIMARY,
              DNS_SECONDARY);

  return true;
}

void WifiManager::reconnectIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.disconnect();
    connectToWifi();
  }
}

bool WifiManager::isConnectedToWifi() { return WiFi.isConnected(); }

void WifiManager::disconnectWifi() {
  WiFi.disconnect(true);
  WiFi.setSleep(true);
  WiFi.mode(WIFI_OFF);
}

void WifiManager::startWifiHotspot() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(hotspotName, hotspotPassword);
}

void WifiManager::stopWifiHotspot() {
  WiFi.softAPdisconnect();
  WiFi.setSleep(true);
  WiFi.mode(WIFI_OFF);
}

String WifiManager::getConfiguredHotspotSsid() { return hotspotName; }
String WifiManager::getConfiguredHotspotPassword() { return hotspotPassword; }
IPAddress WifiManager::getHotspotGatewayIp() { return WiFi.softAPIP(); }

WifiManager wifiManager = WifiManager();
