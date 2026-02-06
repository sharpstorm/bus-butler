#pragma once

#include "ITouchHandler.h"

class IAppState : public ITouchHandler {
 public:
  virtual void enter() = 0;
  virtual void loop() = 0;
  virtual void exit() = 0;
};

enum AppState {
  Init = 0,
  Boot,
  Setup,
  Homescreen,
  Settings,
};

class IAppStateResolver {
 public:
  virtual IAppState* getStateRefById(AppState state);
};
