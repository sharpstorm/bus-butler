#include "HomescreenState.h"

#include "../../Screen/Screen.h"

void HomescreenState::enter() {
  gfx->fillScreen(BLACK);
  gfx->drawRect(SCREEN_X_CENTER - 120, SCREEN_Y_CENTER - 40, 240, 80, MAGENTA);

  gfx->setTextColor(GREEN);
  ScreenRenderer::drawCenterText("Hello!", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER - 20);
}

void HomescreenState::exit() {}
void HomescreenState::loop() {}

void HomescreenState::handleTouch(int x, int y) {}
