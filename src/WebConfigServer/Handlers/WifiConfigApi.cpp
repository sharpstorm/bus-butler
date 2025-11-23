#include "WifiConfigApi.h"

#include "../../Wifi/WifiConfigManager.h"
#include "../HttpConstants.h"
#include "WiFi.h"

String wifiEncryptionTypeToString(wifi_auth_mode_t authMode) {
  switch (authMode) {
    case WIFI_AUTH_OPEN:
      return "open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA+WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-EAP";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2+WPA3";
    case WIFI_AUTH_WAPI_PSK:
      return "WAPI";
    default:
      return "unknown";
  }
}

void WifiConfigApi::scanWifiNetworks(AsyncWebServerRequest* request) {
  AsyncResponseStream* response = request->beginResponseStream(MimeType::JSON);
  int16_t numNetworksFound = WiFi.scanNetworks();

  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  JsonArray networks = root["networks"].to<JsonArray>();

  for (int i = 0; i < numNetworksFound; i++) {
    JsonObject networkRecord = networks.add<JsonObject>();
    networkRecord["ssid"] = WiFi.SSID(i);
    networkRecord["rssi"] = WiFi.RSSI(i);
    networkRecord["security"] =
        wifiEncryptionTypeToString(WiFi.encryptionType(i));
  }

  serializeJson(root, *response);
  request->send(response);
}

void WifiConfigApi::saveWifiNetwork(AsyncWebServerRequest* request,
                                    JsonVariant& jsonBody) {
  JsonObject reqBody = jsonBody.as<JsonObject>();
  if (reqBody["ssid"].isUnbound() || reqBody["passphrase"].isUnbound()) {
    request->send(400, MimeType::JSON, "{ \"success\": false }");
    return;
  }

  wifiConfigManager.putConfig(reqBody["ssid"].as<String>(),
                              reqBody["passphrase"].as<String>());
  request->send(200, MimeType::JSON, "{ \"success\": true }");
}

void WifiConfigApi::getSavedWifiNetwork(AsyncWebServerRequest* request) {
  WifiConfig config = wifiConfigManager.getConfig();

  if (!config.isConfigured) {
    request->send(200, MimeType::JSON, "{ \"ssid\": null }");
    return;
  }

  char respBody[50];
  sprintf(respBody, "{ \"ssid\": \"%s\" }", config.ssid.c_str());
  request->send(200, MimeType::JSON, respBody);
}