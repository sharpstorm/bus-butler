#pragma once

#include "../../Wifi/MDnsClient.h"
#include "../IAppState.h"
#include "../ITouchHandler.h"
#include "BootScreenRenderer.h"

class BootState : public IAppState {
 private:
  BootScreenRenderer renderer;

 public:
  void enter();
  void loop();
  void exit();

  void handleTouch(int x, int y);
  bool isSetupComplete();
};
