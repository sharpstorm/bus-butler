#pragma once

#include <memory>

#include "../../Util/Time.h"
#include "../IAppState.h"

using namespace std;

enum class SettingsInteraction {
  None,
  BrightnessSlider,
};

class SettingsState : public IAppState {
 private:
  uint8_t currentBrightness = 100;
  uint8_t savedBrightness = 100;
  bool isDraggingSlider = false;

  int32_t clearSyncStatusAt = -1;
  bool syncInProgress = false;
  SettingsInteraction activeInteraction = SettingsInteraction::None;

  void triggerTimeSync();
  void updateBrightnessSlider(int x);
  uint8_t toTrueBrightness(uint8_t brightnessPercent);
  uint8_t fromTrueBrightness(uint8_t brightness);

 public:
  void enter();
  void loop();
  void exit();

  void handleTouch(int x, int y);
};
