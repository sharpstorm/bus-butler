#pragma once

#include "IAppState.h"

enum AppState {
  Init = 0,
  Setup = 1,
  Homescreen = 2,
};

extern AppState activeAppState;

void changeAppState(AppState newState);
