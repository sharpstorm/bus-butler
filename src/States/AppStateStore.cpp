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
    case AppState::Homescreen:
      return &homescreenState;
    case AppState::Settings:
      return &settingsState;
    case AppState::WeatherForecast:
      return &weatherForecastState;
    default:
      return nullptr;
  }
}

AppStateStore appStateStore;
