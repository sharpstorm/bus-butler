#include "WebConfigServer.h"

#include "Handlers/StaticServeHandler.h"

WebConfigServer::WebConfigServer() : server(HTTP_PORT) {
  server.on("/", HTTP_GET, StaticServeHandler::handleHTML);
  server.on("/index.js", HTTP_GET, StaticServeHandler::handleJS);
  server.on("/index.css", HTTP_GET, StaticServeHandler::handleCSS);
}

void WebConfigServer::start() { server.begin(); }

void WebConfigServer::stop() { server.end(); }

WebConfigServer webConfigServer;
