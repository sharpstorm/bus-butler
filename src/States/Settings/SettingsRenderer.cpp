#include "SettingsRenderer.h"

#include "../../Screen/Colors.h"
#include "../../Screen/Fonts/RobotoMonoLight32.h"
#include "../../Screen/Screen.h"

void SettingsRenderer::renderSettingsUI() {
  gfx->fillScreen(BG_DARK);

  // Title
  gfx->setFont(&Roboto_Mono_Light_32);
  gfx->setTextSize(1);
  gfx->setTextColor(NEON_CYAN);
  ScreenRenderer::drawCenterText("SETTINGS", SCREEN_WIDTH / 2, 32);
}

void SettingsRenderer::renderBrightnessControl(int curBrightness) {
  const int sliderTrackY = SLIDER_Y + 20;
  const int trackHeight = 6;

  // Clear area
  gfx->fillRect(CONTENT_PADDING_X - 10, SLIDER_Y - 10, CONTENT_WIDTH + 20,
                SLIDER_HEIGHT + 20, BG_DARK);

  // Label
  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->setCursor(CONTENT_PADDING_X, SLIDER_Y);
  gfx->print("Brightness: ");
  gfx->print(curBrightness);
  gfx->print("%");

  // Slider track
  gfx->fillRect(CONTENT_PADDING_X, sliderTrackY, CONTENT_WIDTH, trackHeight,
                GRAY_MID);

  // Slider fill (active part)
  const int fillWidth = (CONTENT_WIDTH * curBrightness) / 100;
  gfx->fillRect(CONTENT_PADDING_X, sliderTrackY, fillWidth, trackHeight,
                NEON_CYAN);

  // Slider thumb
  const int thumbX = CONTENT_PADDING_X + fillWidth - 6;
  gfx->fillCircle(thumbX + 6, sliderTrackY + trackHeight / 2, 10, BG_PANEL);
  gfx->fillCircle(thumbX + 6, sliderTrackY + trackHeight / 2, 8, NEON_CYAN);
  gfx->drawCircle(thumbX + 6, sliderTrackY + trackHeight / 2, 10, WHITE);
}

void SettingsRenderer::renderSyncTimeControl(bool syncInProgress) {
  // Sync button
  uint16_t borderColor = syncInProgress ? GRAY_MID : NEON_MAGENTA;
  uint16_t bgColor = BG_CARD;

  gfx->fillRect(CONTENT_PADDING_X, SYNC_BUTTON_Y, CONTENT_WIDTH, BUTTON_HEIGHT,
                bgColor);
  gfx->drawRect(CONTENT_PADDING_X, SYNC_BUTTON_Y, CONTENT_WIDTH, BUTTON_HEIGHT,
                borderColor);

  // Button label
  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);

  ScreenRenderer::drawCenterText(syncInProgress ? "Syncing..." : "Sync Time",
                                 CONTENT_CENTER,
                                 SYNC_BUTTON_Y + (BUTTON_HEIGHT / 2));
}

void SettingsRenderer::clearSyncTimeStatus() {
  renderSyncTimeStatus("", false);
}

void SettingsRenderer::renderSyncTimeStatus(String message, bool syncSuccess) {
  gfx->fillRect(CONTENT_PADDING_X - 10, SYNC_BUTTON_Y + BUTTON_HEIGHT + 1,
                CONTENT_WIDTH + 20, 60, BG_DARK);

  if (message.length() <= 0) {
    return;
  }

  gfx->setFont(NULL);
  gfx->setTextSize(1);
  gfx->setTextColor(syncSuccess ? NEON_CYAN : STATUS_RED);
  ScreenRenderer::drawCenterText(message, CONTENT_CENTER,
                                 SYNC_BUTTON_Y + BUTTON_HEIGHT + 16);
}

void SettingsRenderer::renderBackButton() {
  // Back button
  gfx->fillRect(CONTENT_PADDING_X, BACK_BUTTON_Y, CONTENT_WIDTH, BUTTON_HEIGHT,
                BG_CARD);
  gfx->drawRect(CONTENT_PADDING_X, BACK_BUTTON_Y, CONTENT_WIDTH, BUTTON_HEIGHT,
                NEON_CYAN);

  // Button label
  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText("< Back", CONTENT_CENTER,
                                 BACK_BUTTON_Y + (BUTTON_HEIGHT / 2));
}
