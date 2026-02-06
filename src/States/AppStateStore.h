#pragma once

#include "Boot/BootState.h"
#include "Homescreen/HomescreenState.h"
#include "IAppState.h"
#include "Settings/SettingsState.h"
#include "Setup/SetupState.h"

/*
  This class is only meant to act as a store, and registers itself with
  AppStateManager to avoid circular dependencies via a function ptr.
*/
class AppStateStore : public IAppStateResolver {
 private:
  BootState bootState;
  SetupState setupState;
  HomescreenState homescreenState;
  SettingsState settingsState;

 public:
  AppStateStore();
  IAppState* getStateRefById(AppState state);
};

extern AppStateStore appStateStore;
