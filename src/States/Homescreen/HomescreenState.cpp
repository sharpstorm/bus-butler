#include "HomescreenState.h"

#include "../../Screen/Fonts/BusTypes.h"
#include "../../Screen/Fonts/RobotoMonoDigits30.h"
#include "../../Screen/Fonts/RobotoMonoDigits40.h"
#include "../../Screen/Screen.h"

void HomescreenState::enter() {
  gfx->fillScreen(BLACK);

  gfx->drawRoundRect(10, 10, 90, 50, 2, MAGENTA);
  gfx->drawRoundRect(10, 75, 90, 50, 2, MAGENTA);
  gfx->drawRoundRect(10, 140, 90, 50, 2, MAGENTA);
  gfx->drawRoundRect(10, 205, 90, 50, 2, MAGENTA);

  gfx->drawRoundRect(250, 10, 90, 50, 2, MAGENTA);
  gfx->drawRoundRect(250, 75, 90, 50, 2, MAGENTA);
  gfx->drawRoundRect(250, 140, 90, 50, 2, MAGENTA);
  gfx->drawRoundRect(250, 205, 90, 50, 2, MAGENTA);

  gfx->setFont(&Roboto_Mono_Light_Digits40);
  gfx->setTextColor(GREEN);
  ScreenRenderer::drawCenterText("157", 50, 35);
  ScreenRenderer::drawCenterText("198", 50, 100);
  ScreenRenderer::drawCenterText("174", 50, 165);
  ScreenRenderer::drawCenterText("172", 50, 230);

  ScreenRenderer::drawCenterText("157", 290, 35);
  ScreenRenderer::drawCenterText("198", 290, 100);
  ScreenRenderer::drawCenterText("174", 290, 165);
  ScreenRenderer::drawCenterText("172", 290, 230);

  gfx->setFont(&Roboto_Mono_Medium_Digits30);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawText("10", 110, 37);
  ScreenRenderer::drawText("15", 110, 102);
  ScreenRenderer::drawText("20", 110, 167);
  ScreenRenderer::drawText("40", 110, 232);

  ScreenRenderer::drawText("20", 165, 37);
  ScreenRenderer::drawText("25", 165, 102);
  ScreenRenderer::drawText("30", 165, 167);
  ScreenRenderer::drawText("50", 165, 232);

  ScreenRenderer::drawText("10", 350, 37);
  ScreenRenderer::drawText("15", 350, 102);
  ScreenRenderer::drawText("20", 350, 167);
  ScreenRenderer::drawText("40", 350, 232);

  ScreenRenderer::drawText("20", 405, 37);
  ScreenRenderer::drawText("25", 405, 102);
  ScreenRenderer::drawText("30", 405, 167);
  ScreenRenderer::drawText("50", 405, 232);

  gfx->setFont(&BusTypesFont);
  gfx->setTextColor(CYAN);
  ScreenRenderer::drawText("a", 110, 8);
  ScreenRenderer::drawText("b", 110, 73);
  ScreenRenderer::drawText("c", 110, 138);
  ScreenRenderer::drawText("a", 110, 203);

  ScreenRenderer::drawText("a", 165, 8);
  ScreenRenderer::drawText("b", 165, 73);
  ScreenRenderer::drawText("c", 165, 138);
  ScreenRenderer::drawText("a", 165, 203);

  ScreenRenderer::drawText("a", 350, 8);
  ScreenRenderer::drawText("b", 350, 73);
  ScreenRenderer::drawText("c", 350, 138);
  ScreenRenderer::drawText("a", 350, 203);

  ScreenRenderer::drawText("a", 405, 8);
  ScreenRenderer::drawText("b", 405, 73);
  ScreenRenderer::drawText("c", 405, 138);
  ScreenRenderer::drawText("a", 405, 203);

  gfx->flush();
}

void HomescreenState::exit() {}
void HomescreenState::loop() {}

void HomescreenState::handleTouch(int x, int y) {}
