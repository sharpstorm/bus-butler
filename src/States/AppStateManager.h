#pragma once

#include "IAppState.h"

class AppStateManager {
 private:
  IAppStateResolver* stateResolver;

  AppState activeState = AppState::Init;
  IAppState* activeStateRef = nullptr;

 public:
  // Meant for internal use during bootstrapping. No need to call under any
  // circumstance
  void registerAppStateResolver(IAppStateResolver* resolver);
  IAppState* getActiveState();
  AppState getActiveStateId();
  void changeState(AppState state);
};

extern AppStateManager appStateManager;
