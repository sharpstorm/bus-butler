#include "WebConfigServer.h"

#include "Handlers/StaticServeHandler.h"
#include "Handlers/WifiConfigApi.h"

WebConfigServer::WebConfigServer() : server(HTTP_PORT) {
  server.on("/", HTTP_GET, StaticServeHandler::handleHTML);
  server.on("/index.js", HTTP_GET, StaticServeHandler::handleJS);
  server.on("/index.css", HTTP_GET, StaticServeHandler::handleCSS);

  server.on("/api/wifi/scan", HTTP_GET, WifiConfigApi::scanWifiNetworks);
  // TODO: Fix linker error
  // server.on("/api/wifi/config", HTTP_POST, WifiConfigApi::saveWifiNetwork);
  server.on("/api/wifi/config", HTTP_GET, WifiConfigApi::getSavedWifiNetwork);
}

void WebConfigServer::start() { server.begin(); }

void WebConfigServer::stop() { server.end(); }

WebConfigServer webConfigServer;
