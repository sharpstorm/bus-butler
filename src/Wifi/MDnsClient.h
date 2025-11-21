#pragma once

#define MDNS_HOSTNAME "busbutler"

class MDnsClient {
 public:
  void start();
  void stop();
};