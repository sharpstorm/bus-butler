#pragma once

#include "Boot/BootState.h"
#include "Controls/ControlsState.h"
#include "Home/HomeState.h"
#include "IAppState.h"
#include "Mrt/MrtPageState.h"
#include "Notify/NotifyState.h"
#include "Setup/SetupState.h"
#include "Status/StatusPageState.h"
#include "StopQr/StopQrState.h"
#include "Weather/WeatherPageState.h"

/*
  This class is only meant to act as a store, and registers itself with
  AppStateManager to avoid circular dependencies via a function ptr.
*/
class AppStateStore : public IAppStateResolver {
 private:
  BootState bootState;
  SetupState setupState;
  HomeState homeState;
  WeatherPageState weatherPageState;
  MrtPageState mrtPageState;
  StatusPageState statusPageState;
  StopQrState stopQrState;
  ControlsState controlsState;

 public:
  AppStateStore();
  IAppState* getStateRefById(AppState state);
};

extern AppStateStore appStateStore;
