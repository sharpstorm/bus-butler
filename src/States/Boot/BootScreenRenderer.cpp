#include "BootScreenRenderer.h"

#include "../../Screen/Screen.h"

BootScreenRenderer::BootScreenRenderer() {}

void BootScreenRenderer::renderInitial() {
  gfx->fillScreen(BLACK);
  gfx->drawRect(SCREEN_X_CENTER - 120, SCREEN_Y_CENTER - 40, 240, 100, MAGENTA);

  gfx->setTextColor(GREEN);
  ScreenRenderer::drawCenterText("Hello!", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER - 20);
  gfx->drawFastHLine(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER - 10, 220, WHITE);

  gfx->setCursor(SCREEN_X_CENTER - 80, SCREEN_Y_CENTER);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText("Bus butler is starting", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 10);
  ScreenRenderer::drawCenterText("Connecting to WiFi", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 25);
  gfx->flush();
}

void BootScreenRenderer::renderWifiConnected() {
  gfx->fillRect(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 18, 220, 14, BLACK);
  gfx->setTextColor(GREEN);
  ScreenRenderer::drawCenterText("Connected to WiFi", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 25);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText("Starting", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 40);
  gfx->flush();
}
