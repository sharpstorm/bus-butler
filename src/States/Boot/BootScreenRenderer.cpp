#include "BootScreenRenderer.h"

#include "../../Screen/Screen.h"

BootScreenRenderer::BootScreenRenderer() {}

void BootScreenRenderer::renderInitial() {
  gfx->fillScreen(BLACK);
  gfx->drawRect(SCREEN_X_CENTER - 120, SCREEN_Y_CENTER - 40, 240, 120, MAGENTA);

  gfx->setTextColor(GREEN);
  ScreenRenderer::drawCenterText("Hello!", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER - 20);
  gfx->drawFastHLine(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER - 10, 220, WHITE);

  gfx->setCursor(SCREEN_X_CENTER - 80, SCREEN_Y_CENTER);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText("Bus butler is starting", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 10);
  gfx->flush();
}

void BootScreenRenderer::renderWifiConnecting(int attempt, int maxAttempts) {
  gfx->fillRect(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 18, 220, 14, BLACK);
  gfx->setTextColor(YELLOW);
  String statusText = "Connecting WiFi... (" + String(attempt) + "/" +
                      String(maxAttempts) + ")";
  ScreenRenderer::drawCenterText(statusText, SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 25);
  gfx->flush();
}

void BootScreenRenderer::renderWifiConnected() {
  gfx->fillRect(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 18, 220, 14, BLACK);
  gfx->setTextColor(GREEN);
  ScreenRenderer::drawCenterText("Connected to WiFi", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 25);
  gfx->flush();
}

void BootScreenRenderer::renderWifiFailed() {
  gfx->fillRect(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 18, 220, 28, BLACK);
  gfx->setTextColor(RED);
  ScreenRenderer::drawCenterText("WiFi Failed", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 25);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText("Returning to setup", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 40);
  gfx->flush();
}

void BootScreenRenderer::renderWaitInternet() {
  gfx->fillRect(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 33, 220, 14, BLACK);
  gfx->setTextColor(YELLOW);
  ScreenRenderer::drawCenterText("Waiting for internet", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 40);
  gfx->flush();
}

void BootScreenRenderer::renderInternetConnected() {
  gfx->fillRect(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 33, 220, 14, BLACK);
  gfx->setTextColor(GREEN);
  ScreenRenderer::drawCenterText("Connected to internet", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 40);
  gfx->flush();
}

void BootScreenRenderer::renderSyncingTime() {
  gfx->fillRect(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 48, 220, 14, BLACK);
  gfx->setTextColor(YELLOW);
  ScreenRenderer::drawCenterText("Syncing time", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 55);
  gfx->flush();
}

void BootScreenRenderer::renderStarting() {
  gfx->fillRect(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 48, 220, 14, BLACK);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText("Starting system", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 55);
  gfx->flush();
}
