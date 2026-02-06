#pragma once

#include <memory>

#include "../../Screen/ScreenReference.h"
#include "../../Util/Time.h"
#include "../IAppState.h"

class SettingsRenderer {
 public:
  // UI update functions
  static void renderSettingsUI();
  static void renderBrightnessControl(int curBrightness);
  static void renderSyncTimeControl(bool syncInProgress);
  static void clearSyncTimeStatus();
  static void renderSyncTimeStatus(String message, bool syncSuccess);
  static void renderBackButton();

  // UI constants
  static constexpr int SLIDER_Y = 80;
  static constexpr int SLIDER_HEIGHT = 40;
  static constexpr int SYNC_BUTTON_Y = 150;
  static constexpr int BACK_BUTTON_Y = 220;
  static constexpr int BUTTON_HEIGHT = 36;
  static constexpr int CONTENT_PADDING_X = 40;
  static constexpr int CONTENT_WIDTH = SCREEN_WIDTH - 2 * CONTENT_PADDING_X;
  static constexpr int CONTENT_CENTER = CONTENT_PADDING_X + (CONTENT_WIDTH / 2);
};
