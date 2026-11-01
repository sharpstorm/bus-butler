#pragma once

#include <Arduino.h>

#include "../Config/ConfigManager.h"

/*
  Decides the backlight level:
    - the user's brightness (controls panel / web portal)
    - lowered to the night level inside the night schedule window
    - lowered further to the idle level after N minutes without a touch
  A touch on an idle-dimmed screen only wakes it; that touch is swallowed so
  it doesn't also press whatever was under the finger.
*/
class DisplayPower {
 public:
  void begin();
  void loop();

  // Call on every touch. Returns true if the screen was idle-dimmed and this
  // touch should only wake it.
  bool onTouch();

  // Something wants attention (e.g. a notification): end idle dimming as a
  // touch would, without the next touch being swallowed.
  void wake();
  // While boosted the backlight ignores night and idle dimming (full-screen
  // notifications), so a doorbell at night is readable.
  void setBoost(bool on);
  bool isBoosted() { return boost; }

  // Live brightness while dragging the slider; endPreview() saves it.
  void preview(uint8_t brightness);
  void endPreview(bool save);

  const DisplayConfig& getConfig() { return config; }
  void saveConfig(const DisplayConfig& next);

  bool isIdleDimmed() { return idleDimmed; }
  // Night dimming is lowering the brightness right now
  bool isNightActive() { return nightActive; }
  // Inside the night window with warm colours turned on
  bool wantsNightPalette() { return nightPalette; }
  uint8_t getAppliedLevel() { return applied; }

 private:
  DisplayConfig config;
  uint32_t configRevision = 0;
  uint32_t lastTouchMs = 0;
  uint32_t lastEvalMs = 0;
  bool idleDimmed = false;
  bool nightActive = false;
  bool nightPalette = false;
  int16_t previewLevel = -1;
  bool boost = false;
  int16_t applied = -1;

  // Backlight fade
  float fadeFrom = 0;
  int16_t fadeTarget = -1;
  uint32_t fadeStartMs = 0;
  uint16_t fadeMs = 0;
  int16_t shown = -1;  // level currently on the backlight

  void reloadIfChanged();
  bool inNightWindow();
  // Pick the target level; fadeDuration is used if it changes.
  void apply(uint16_t fadeDuration, bool force = false);
  void stepFade();
};

extern DisplayPower displayPower;
