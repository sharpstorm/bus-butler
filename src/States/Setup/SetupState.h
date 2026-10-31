#pragma once

#include "../../Wifi/MDnsClient.h"
#include "../IAppState.h"
#include "SetupScreenRenderer.h"

class SetupState : public IAppState {
 private:
  SetupScreenRenderer setupRenderer;

 public:
  void enter() override;
  void loop() override;
  void exit() override;

  bool isSetupComplete();
};
