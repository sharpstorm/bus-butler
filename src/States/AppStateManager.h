#pragma once

#include "AppStateStore.h"
#include "Homescreen/HomescreenState.h"
#include "Setup/SetupState.h"

class AppStateManager {
 private:
  AppState activeState = AppState::Init;
  IAppState* activeStateRef = nullptr;

  SetupState setupState;
  HomescreenState homescreenState;

  IAppState* getStateRefById(AppState state);

 public:
  IAppState* getActiveState();
};
