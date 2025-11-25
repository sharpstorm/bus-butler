#include "AppStateManager.h"

IAppState* AppStateManager::getActiveState() {
  if (activeState != activeAppState) {
    activeState = activeAppState;
    activeStateRef = getStateRefById(activeState);
  }
  return activeStateRef;
}

IAppState* AppStateManager::getStateRefById(AppState state) {
  switch (state) {
    case AppState::Setup:
      return &setupState;
    case AppState::Homescreen:
      return &homescreenState;
    default:
      return nullptr;
  }
}
