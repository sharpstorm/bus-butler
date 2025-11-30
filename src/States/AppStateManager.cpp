#include "AppStateManager.h"

IAppState* AppStateManager::getActiveState() { return activeStateRef; }

AppState AppStateManager::getActiveStateId() { return activeState; }

void AppStateManager::changeState(AppState state) {
  if (activeStateRef != nullptr) {
    activeStateRef->exit();
  }

  activeState = state;
  activeStateRef = stateResolver->getStateRefById(state);

  if (activeStateRef != nullptr) {
    activeStateRef->enter();
  }
}

void AppStateManager::registerAppStateResolver(IAppStateResolver* resolver) {
  stateResolver = resolver;
}

AppStateManager appStateManager;
