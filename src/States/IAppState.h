#pragma once

#include "ITouchHandler.h"

class IAppState : public ITouchHandler {
 public:
  virtual void enter();
  virtual void loop();
  virtual void exit();
};

enum AppState {
  Init = 0,
  Boot,
  Setup,
  Homescreen,
};

class IAppStateResolver {
 public:
  virtual IAppState* getStateRefById(AppState state);
};
