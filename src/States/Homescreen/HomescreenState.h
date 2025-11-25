#pragma once

#include "../../Wifi/MDnsClient.h"
#include "../IAppState.h"

class HomescreenState : public IAppState {
 private:
 public:
  void enter();
  void loop();
  void exit();

  void handleTouch(int x, int y);
};
