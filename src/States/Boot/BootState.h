#pragma once

#include "../../Wifi/MDnsClient.h"
#include "../IAppState.h"
#include "BootScreenRenderer.h"

class BootState : public IAppState {
 private:
  BootScreenRenderer renderer;

  bool isInternetConnected();

 public:
  void enter() override;
  void loop() override;
  void exit() override;

  bool isSetupComplete();
};
