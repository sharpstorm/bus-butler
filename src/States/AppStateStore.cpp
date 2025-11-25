#include "AppStateStore.h"

AppState activeAppState = AppState::Init;

void changeAppState(AppState newState) { activeAppState = newState; }
