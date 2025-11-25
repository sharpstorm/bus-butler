#pragma once

#include "../../Wifi/MDnsClient.h"
#include "../IAppState.h"
#include "../ITouchHandler.h"
#include "SetupScreenRenderer.h"

class SetupState : public IAppState {
 private:
  SetupScreenRenderer setupRenderer;
  MDnsClient mdnsClient;

 public:
  void enter();
  void loop();
  void exit();

  void handleTouch(int x, int y);
  bool isSetupComplete();
};
