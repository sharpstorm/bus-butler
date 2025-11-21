#pragma once

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

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
