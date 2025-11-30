#include "SetupScreenRenderer.h"

#include "../../Screen/Screen.h"
#include "../../Wifi/WifiManager.h"

SetupScreenRenderer::SetupScreenRenderer() : isPainted(false) {}

void SetupScreenRenderer::render() {
  if (isPainted) {
    return;
  }

  gfx->fillScreen(BLACK);
  gfx->drawRect(SCREEN_X_CENTER - 120, SCREEN_Y_CENTER - 40, 240, 80, MAGENTA);

  gfx->setTextColor(GREEN);
  ScreenRenderer::drawCenterText("Hello!", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER - 20);
  gfx->drawFastHLine(SCREEN_X_CENTER - 110, SCREEN_Y_CENTER - 10, 220, WHITE);

  gfx->setCursor(SCREEN_X_CENTER - 80, SCREEN_Y_CENTER);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText("Bus butler is unconfigured", SCREEN_X_CENTER,
                                 SCREEN_Y_CENTER + 10);
  ScreenRenderer::drawCenterText("Connect to WiFi network below",
                                 SCREEN_X_CENTER, SCREEN_Y_CENTER + 25);

  gfx->setTextColor(GREEN);
  ScreenRenderer::drawText("SSID: ", SCREEN_X_CENTER - 150,
                           SCREEN_Y_CENTER + 60);
  ScreenRenderer::drawText("Password: ", SCREEN_X_CENTER, SCREEN_Y_CENTER + 60);

  gfx->setTextColor(WHITE);
  ScreenRenderer::drawText(wifiManager.getConfiguredHotspotSsid().c_str(),
                           SCREEN_X_CENTER - 110, SCREEN_Y_CENTER + 60);
  ScreenRenderer::drawText(wifiManager.getConfiguredHotspotPassword().c_str(),
                           SCREEN_X_CENTER + 60, SCREEN_Y_CENTER + 60);

  gfx->flush();
  isPainted = true;
}
