#include "WebConfigServer.h"

#include "Handlers/ConfigTestApi.h"
#include "Handlers/HardwareConfigApi.h"
#include "Handlers/LtaDatamallConfigApi.h"
#include "Handlers/StaticServeHandler.h"
#include "Handlers/WifiConfigApi.h"

WebConfigServer::WebConfigServer() : server(HTTP_PORT) {
  server.on("/", HTTP_GET, StaticServeHandler::handleHTML);
  server.on("/index.js", HTTP_GET, StaticServeHandler::handleJS);
  server.on("/index.css", HTTP_GET, StaticServeHandler::handleCSS);

  server.on("/api/wifi/scan", HTTP_GET, WifiConfigApi::scanWifiNetworks);
  server.on("/api/wifi/config", HTTP_POST, WifiConfigApi::saveWifiNetwork);
  server.on("/api/wifi/config", HTTP_GET, WifiConfigApi::getSavedWifiNetwork);

  server.on("/api/config/lta-api", HTTP_GET,
            LtaDatamallConfigApi::getSavedApiKey);
  server.on("/api/config/lta-api", HTTP_POST, LtaDatamallConfigApi::saveApiKey);
  server.on("/api/config/busstop-code", HTTP_GET,
            LtaDatamallConfigApi::getSavedBusStopCode);
  server.on("/api/config/busstop-code", HTTP_POST,
            LtaDatamallConfigApi::saveBusStopCode);

  server.on("/api/config/screen-brightness", HTTP_POST,
            HardwareConfigApi::setScreenBrightness);
  server.on("/api/config/screen-brightness", HTTP_GET,
            HardwareConfigApi::getScreenBrightness);

  server.on("/api/test/bus-arrival", HTTP_POST, ConfigTestApi::queryBusArrival);
}

void WebConfigServer::start() { server.begin(); }

void WebConfigServer::stop() { server.end(); }

WebConfigServer webConfigServer;
