#pragma once

#include <Arduino.h>

// EspAsyncWebServer Imports
#include <AsyncTCP.h>
#include <WiFi.h>

#include "ArduinoJson.h"

// Web Server
#include <EspAsyncWebServer.h>

#include "AsyncJson.h"

using namespace std;

#define HTTP_PORT 80

class WebConfigServer {
 private:
  AsyncWebServer server;

 public:
  WebConfigServer();
  void start();
  void stop();
};

extern WebConfigServer webConfigServer;
