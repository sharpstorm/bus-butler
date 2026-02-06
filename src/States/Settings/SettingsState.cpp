#include "SettingsState.h"

#include "../../Config/ConfigManager.h"
#include "../../Screen/Colors.h"
#include "../../Screen/Fonts/RobotoMonoLight32.h"
#include "../../Screen/Screen.h"
#include "../AppStateManager.h"
#include "SettingsRenderer.h"

void SettingsState::enter() {
  gfx->fillScreen(BG_DARK);

  // Load current brightness from config and screen
  currentBrightness = configManager.getBrightness();
  if (currentBrightness == 0) {
    currentBrightness = 100;
  }

  savedBrightness = currentBrightness;
  screen.setBrightness(currentBrightness);

  isDraggingSlider = false;
  syncInProgress = false;
  clearSyncStatusAt = -1;

  SettingsRenderer::renderSettingsUI();
  SettingsRenderer::renderBrightnessControl(currentBrightness);
  SettingsRenderer::renderSyncTimeControl(false);
  SettingsRenderer::renderBackButton();
  gfx->flush();
}

void SettingsState::exit() {
  // Save brightness to config if changed
  if (currentBrightness != savedBrightness) {
    configManager.saveBrightness(currentBrightness);
  }
}

void SettingsState::loop() {
  // Clear sync message after 3 seconds
  if (clearSyncStatusAt > 0 && !syncInProgress) {
    long timeNow = TimeUtil::getUnixTime();
    if (timeNow > clearSyncStatusAt) {
      SettingsRenderer::renderSyncTimeControl(syncInProgress);
      SettingsRenderer::clearSyncTimeStatus();
      gfx->flush();
      clearSyncStatusAt = -1;
    }
  }
}

void SettingsState::handleTouch(int x, int y) {
  // Back button
  if (isInHitbox(x, y, SettingsRenderer::CONTENT_PADDING_X,
                 SettingsRenderer::BACK_BUTTON_Y,
                 SettingsRenderer::CONTENT_WIDTH,
                 SettingsRenderer::BUTTON_HEIGHT)) {
    appStateManager.changeState(AppState::Homescreen);
    return;
  }

  // Sync time button
  if (isInHitbox(x, y, SettingsRenderer::CONTENT_PADDING_X,
                 SettingsRenderer::SYNC_BUTTON_Y,
                 SettingsRenderer::CONTENT_WIDTH,
                 SettingsRenderer::BUTTON_HEIGHT)) {
    if (!syncInProgress) {
      triggerTimeSync();
    }
    return;
  }

  // Brightness slider
  if (isInHitbox(x, y, SettingsRenderer::CONTENT_PADDING_X,
                 SettingsRenderer::SLIDER_Y, SettingsRenderer::CONTENT_WIDTH,
                 SettingsRenderer::SLIDER_HEIGHT)) {
    updateBrightnessSlider(x);
    isDraggingSlider = true;
    activeInteraction = SettingsInteraction::BrightnessSlider;
    return;
  }

  // Release slider
  if (isDraggingSlider &&
      activeInteraction == SettingsInteraction::BrightnessSlider) {
    isDraggingSlider = false;
    activeInteraction = SettingsInteraction::None;
  }
}

void SettingsState::triggerTimeSync() {
  syncInProgress = true;
  SettingsRenderer::renderSyncTimeControl(syncInProgress);
  SettingsRenderer::clearSyncTimeStatus();
  gfx->flush();

  // Perform NTP sync
  bool result = TimeUtil::syncer.syncNtpTime();
  syncInProgress = false;
  String syncMessage =
      result ? "Time synced successfully" : "Sync failed - check WiFi";
  clearSyncStatusAt = TimeUtil::getUnixTime() + 3;  // 3 Seconds

  SettingsRenderer::renderSyncTimeControl(syncInProgress);
  SettingsRenderer::renderSyncTimeStatus(syncMessage, result);
  gfx->flush();
}

void SettingsState::updateBrightnessSlider(int x) {
  // Calculate brightness from touch position
  int sliderX = x - SettingsRenderer::CONTENT_PADDING_X;
  sliderX = max(0, min(SettingsRenderer::CONTENT_WIDTH, sliderX));

  uint8_t newBrightness = (sliderX * 100) / SettingsRenderer::CONTENT_WIDTH;
  newBrightness =
      max((uint8_t)10, min((uint8_t)100, newBrightness));  // Clamp to 10-100%

  if (newBrightness != currentBrightness) {
    currentBrightness = newBrightness;
    screen.setBrightness(currentBrightness);
    SettingsRenderer::renderBrightnessControl(currentBrightness);
    gfx->flush();
  }
}