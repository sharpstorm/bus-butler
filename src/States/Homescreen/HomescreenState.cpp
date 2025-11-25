#include "HomescreenState.h"

#include "../../Screen/Screen.h"

void HomescreenState::enter() {
  gfx->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
}

void HomescreenState::exit() {}
void HomescreenState::loop() {}

void HomescreenState::handleTouch(int x, int y) {}
