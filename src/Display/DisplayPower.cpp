#include "DisplayPower.h"

#include <time.h>

#include "../Screen/Screen.h"

// Fade lengths (ms)
#define FADE_WAKE 150
#define FADE_CHANGE 300
#define FADE_IDLE 1200
#define FADE_NIGHT 2500

void DisplayPower::begin() {
  config = configManager.getDisplayConfig();
  configRevision = configManager.getDisplayRevision();
  lastTouchMs = millis();
  apply(0, true);
}

void DisplayPower::reloadIfChanged() {
  if (configRevision != configManager.getDisplayRevision()) {
    config = configManager.getDisplayConfig();
    configRevision = configManager.getDisplayRevision();
  }
}

// The window is shared by night dimming and night colours.
bool DisplayPower::inNightWindow() {
  if (!config.nightEnabled && !config.nightTheme) return false;
  struct tm now;
  if (!getLocalTime(&now, 0) || now.tm_year < (2024 - 1900)) {
    return false;  // Clock not synced yet
  }
  uint16_t minutes = now.tm_hour * 60 + now.tm_min;
  if (config.nightStart == config.nightEnd) return false;
  if (config.nightStart < config.nightEnd) {
    return minutes >= config.nightStart && minutes < config.nightEnd;
  }
  // Window crosses midnight, e.g. 23:00 -> 07:00
  return minutes >= config.nightStart || minutes < config.nightEnd;
}

void DisplayPower::apply(uint16_t fadeDuration, bool force) {
  uint8_t level = config.brightness;
  bool wasNight = nightActive;
  bool window = inNightWindow();
  nightActive = window && config.nightEnabled;
  nightPalette = window && config.nightTheme;
  if (nightActive) {
    level = min(level, config.nightLevel);
  }
  if (idleDimmed) {
    level = min(level, config.idleDimLevel);
  }
  if (nightActive != wasNight) {
    fadeDuration = max<uint16_t>(fadeDuration, FADE_NIGHT);
  }
  if (previewLevel >= 0) {
    level = previewLevel;
    fadeDuration = 0;  // Follow the finger exactly
  }
  if (!force && level == applied) return;
  applied = level;

  if (fadeDuration == 0 || shown < 0) {
    fadeTarget = -1;
    shown = level;
    screen.setBrightness(level);
    return;
  }
  fadeFrom = shown;
  fadeTarget = level;
  fadeStartMs = millis();
  fadeMs = fadeDuration;
}

// Called every loop: moves the backlight towards the target.
void DisplayPower::stepFade() {
  if (fadeTarget < 0) return;
  float t = min(1.0f, (millis() - fadeStartMs) / (float)fadeMs);
  float eased = t * t * (3 - 2 * t);  // smoothstep
  int16_t level = lroundf(fadeFrom + (fadeTarget - fadeFrom) * eased);
  if (level != shown) {
    shown = level;
    screen.setBrightness(level);
  }
  if (t >= 1.0f) fadeTarget = -1;
}

void DisplayPower::loop() {
  stepFade();

  uint32_t now = millis();
  if (now - lastEvalMs < 500) return;
  lastEvalMs = now;

  reloadIfChanged();
  bool wasIdle = idleDimmed;
  if (config.idleDimEnabled && previewLevel < 0 &&
      now - lastTouchMs > (uint32_t)config.idleDimMinutes * 60000UL) {
    idleDimmed = true;
  }
  if (!config.idleDimEnabled) {
    idleDimmed = false;
  }
  apply(idleDimmed && !wasIdle ? FADE_IDLE : FADE_CHANGE);
}

bool DisplayPower::onTouch() {
  lastTouchMs = millis();
  if (idleDimmed) {
    idleDimmed = false;
    apply(FADE_WAKE);
    return true;
  }
  return false;
}

void DisplayPower::preview(uint8_t brightness) {
  previewLevel = brightness;
  apply(0);
}

void DisplayPower::endPreview(bool save) {
  if (save && previewLevel >= 0) {
    config.brightness = previewLevel;
    configManager.saveBrightness(previewLevel);
    configRevision = configManager.getDisplayRevision();
  }
  previewLevel = -1;
  apply(0);
}

void DisplayPower::saveConfig(const DisplayConfig& next) {
  configManager.saveDisplayConfig(next);
  config = next;
  configRevision = configManager.getDisplayRevision();
  apply(FADE_CHANGE);
}

DisplayPower displayPower;
