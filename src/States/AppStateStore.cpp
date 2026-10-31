#include "AppStateStore.h"

#include "AppStateManager.h"

AppStateStore::AppStateStore() {
  appStateManager.registerAppStateResolver(this);
}

IAppState* AppStateStore::getStateRefById(AppState state) {
  switch (state) {
    case AppState::Boot:
      return &bootState;
    case AppState::Setup:
      return &setupState;
    case AppState::Home:
      return &homeState;
    case AppState::WeatherPage:
      return &weatherPageState;
    case AppState::MrtPage:
      return &mrtPageState;
    case AppState::StatusPage:
      return &statusPageState;
    case AppState::StopQr:
      return &stopQrState;
    case AppState::Controls:
      return &controlsState;
    default:
      return nullptr;
  }
}

AppStateStore appStateStore;
