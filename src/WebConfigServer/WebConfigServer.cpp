#include "WebConfigServer.h"

#include "Handlers/BusServiceConfigApi.h"
#include "Handlers/ConfigTestApi.h"
#include "Handlers/HardwareConfigApi.h"
#include "Handlers/LtaDatamallConfigApi.h"
#include "Handlers/StaticServeHandler.h"
#include "Handlers/WeatherConfigApi.h"
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
  server.on("/api/config/bus-service-order", HTTP_GET,
            BusServiceConfigApi::getBusServiceOrder);
  server.on(
      "/api/config/bus-service-order", HTTP_POST,
      [](AsyncWebServerRequest* request) {}, NULL,
      BusServiceConfigApi::saveBusServiceOrder);

  server.on("/api/config/screen-brightness", HTTP_POST,
            HardwareConfigApi::setScreenBrightness);
  server.on("/api/config/screen-brightness", HTTP_GET,
            HardwareConfigApi::getScreenBrightness);

  server.on("/api/config/weather-region", HTTP_POST,
            WeatherConfigApi::saveWeatherRegion);
  server.on("/api/config/weather-region", HTTP_GET,
            WeatherConfigApi::getWeatherRegion);
  server.on("/api/config/weather-town", HTTP_POST,
            WeatherConfigApi::saveWeatherTown);
  server.on("/api/config/weather-town", HTTP_GET,
            WeatherConfigApi::getWeatherTown);
  server.on("/api/config/weather-region-list", HTTP_GET,
            WeatherConfigApi::listWeatherRegions);
  server.on("/api/config/weather-town-list", HTTP_GET,
            WeatherConfigApi::listWeatherTowns);

  server.on("/api/test/bus-arrival", HTTP_POST, ConfigTestApi::queryBusArrival);
  server.on("/api/test/weather", HTTP_POST, ConfigTestApi::queryWeather);
}

void WebConfigServer::start() { server.begin(); }

void WebConfigServer::stop() { server.end(); }

WebConfigServer webConfigServer;
