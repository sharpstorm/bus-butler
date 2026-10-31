#include "ControlsState.h"

#include <WiFi.h>

#include "../../Data/DataStore.h"
#include "../../Display/DisplayPower.h"
#include "../../Screen/Screen.h"
#include "../../Ui/Anim.h"
#include "../../Ui/Ui.h"
#include "../../Util/Time.h"
#include "../AppStateManager.h"
#include "../StopQr/StopQrState.h"

#define MIN_BRIGHTNESS 8  // keep the slider from turning the screen off

enum Tile { TileAutoDim = 0, TileNight, TileRefresh, TilePortal, TileCount };

static String hhmm(uint16_t minutes) {
  char buf[6];
  snprintf(buf, sizeof(buf), "%02u:%02u", minutes / 60, minutes % 60);
  return buf;
}

void ControlsState::enter() {
  dragging = false;
  brightnessChanged = false;
  brightness = displayPower.getConfig().brightness;
  refreshFeedbackUntil = 0;
  // Leave the page underneath visible but dimmed
  ui::dim(CONTROLS_H, SCREEN_HEIGHT - CONTROLS_H);
  render();
}

void ControlsState::exit() {
  if (brightnessChanged) {
    displayPower.endPreview(true);
  }
}

void ControlsState::loop() {
  struct tm t;
  int minute = TimeUtil::localTime(t) ? t.tm_min : -1;
  bool feedbackExpired =
      refreshFeedbackUntil != 0 && millis() > refreshFeedbackUntil;
  if (feedbackExpired) refreshFeedbackUntil = 0;
  if (minute != lastMinute || feedbackExpired) render();
}

void ControlsState::render() {
  gfx->fillRect(0, 0, SCREEN_WIDTH, CONTROLS_H, BG_PANEL);
  gfx->drawFastHLine(0, CONTROLS_H - 2, SCREEN_WIDTH, DIM_MAGENTA);
  gfx->drawFastHLine(0, CONTROLS_H - 1, SCREEN_WIDTH, NEON_MAGENTA);

  // Time + network
  struct tm t;
  if (TimeUtil::localTime(t)) {
    lastMinute = t.tm_min;
    int hour12 = t.tm_hour % 12 == 0 ? 12 : t.tm_hour % 12;
    char clock[12];
    snprintf(clock, sizeof(clock), "%d:%02d %s", hour12, t.tm_min,
             t.tm_hour >= 12 ? "pm" : "am");
    ui::text(&FigMed, clock, 16, 26, TEXT_MAIN);
  }
  String net = WiFi.isConnected() ? WiFi.SSID() : String("Offline");
  ui::text(&SgSm, net, 464, 24, GRAY_LIGHT, ui::Right);
  ui::text(&IconSm, ICON_WIFI, 464 - ui::width(&SgSm, net) - 22, 28,
           WiFi.isConnected() ? NEON_CYAN : STATUS_RED);

  renderSlider();
  renderTiles();

  // Grab handle hint
  gfx->fillRoundRect(SCREEN_WIDTH / 2 - 20, CONTROLS_H - 10, 40, 4, 2,
                     GRAY_MID);
  anim::present();
}

void ControlsState::renderSlider() {
  gfx->fillRect(0, SLIDER_Y - 14, SCREEN_WIDTH, SLIDER_H + 28, BG_PANEL);
  ui::text(&IconMd, ICON_BRIGHTNESS, 16, SLIDER_Y + 20, NEON_YELLOW);

  gfx->fillRoundRect(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, SLIDER_H / 2,
                     BG_HIGHLIGHT);
  int16_t fill = (int32_t)SLIDER_W * brightness / 255;
  if (fill >= SLIDER_H) {
    gfx->fillRoundRect(SLIDER_X, SLIDER_Y, fill, SLIDER_H, SLIDER_H / 2,
                       NEON_CYAN);
  }
  int16_t knobX = SLIDER_X + max<int16_t>(SLIDER_H / 2, min<int16_t>(fill, SLIDER_W - SLIDER_H / 2));
  gfx->fillCircle(knobX, SLIDER_Y + SLIDER_H / 2, SLIDER_H / 2 + 3, TEXT_MAIN);

  ui::text(&SgMed, String(brightness * 100 / 255) + "%", 464,
           SLIDER_Y + 16, TEXT_MAIN, ui::Right);
}

void ControlsState::renderTiles() {
  const DisplayConfig& config = displayPower.getConfig();
  for (int i = 0; i < TileCount; i++) {
    int16_t x = TILE_X0 + i * (TILE_W + TILE_GAP);
    bool active = false;
    const char* icon = "";
    String label, detail;
    switch (i) {
      case TileAutoDim:
        active = config.idleDimEnabled;
        icon = ICON_TIMER;
        label = "Auto-dim";
        detail = active ? "After " + String(config.idleDimMinutes) + " min"
                        : String("Off");
        break;
      case TileNight:
        active = config.nightEnabled;
        icon = ICON_BEDTIME;
        label = "Night dim";
        detail = active ? hhmm(config.nightStart) + "-" + hhmm(config.nightEnd)
                        : String("Off");
        break;
      case TileRefresh:
        active = refreshFeedbackUntil != 0;
        icon = ICON_REFRESH;
        label = "Refresh";
        detail = active ? String("Requested") : String("Data + clock");
        break;
      case TilePortal:
        icon = ICON_QR;
        label = "Web portal";
        detail = "Show QR";
        break;
    }
    gfx->fillRoundRect(x, TILE_Y, TILE_W, TILE_H, 8,
                       active ? BG_HIGHLIGHT : BG_CARD);
    if (active) gfx->drawRoundRect(x, TILE_Y, TILE_W, TILE_H, 8, NEON_CYAN);
    ui::text(&IconLg, icon, x + 10, TILE_Y + 36,
             active ? NEON_CYAN : GRAY_LIGHT);
    ui::text(&FigSm, label, x + 10, TILE_Y + 54, TEXT_MAIN);
    ui::text(&SgSm, ui::fit(&SgSm, detail, TILE_W - 16), x + 10, TILE_Y + 69,
             active ? NEON_CYAN : GRAY_LIGHT);
  }
}

int ControlsState::tileAt(int16_t x, int16_t y) {
  if (y < TILE_Y || y >= TILE_Y + TILE_H) return -1;
  for (int i = 0; i < TileCount; i++) {
    int16_t tx = TILE_X0 + i * (TILE_W + TILE_GAP);
    if (x >= tx && x < tx + TILE_W) return i;
  }
  return -1;
}

void ControlsState::setFromX(int16_t x) {
  int32_t value = (int32_t)(x - SLIDER_X) * 255 / SLIDER_W;
  value = constrain(value, MIN_BRIGHTNESS, 255);
  if (value == brightness) return;
  brightness = value;
  brightnessChanged = true;
  displayPower.preview(brightness);
  renderSlider();
  anim::presentRows(SLIDER_Y - 14, SLIDER_H + 28);
}

void ControlsState::onTouchDown(int16_t x, int16_t y) {
  if (y >= SLIDER_Y - 16 && y < SLIDER_Y + SLIDER_H + 16 &&
      x >= SLIDER_X - 12 && x < SLIDER_X + SLIDER_W + 12) {
    dragging = true;
    setFromX(x);
  }
}

void ControlsState::onTouchMove(int16_t x, int16_t y) {
  if (dragging) setFromX(x);
}

void ControlsState::onTouchUp(int16_t x, int16_t y) {
  if (dragging) {
    dragging = false;
  }
}

bool ControlsState::onGesture(const Gesture& g) {
  if (g.startsIn(0, SLIDER_Y - 16, SCREEN_WIDTH, SLIDER_H + 32)) {
    return true;  // The slider owns drags in its row
  }
  if (g.type == GestureType::SwipeUp ||
      (g.type == GestureType::Tap && g.startY >= CONTROLS_H)) {
    appStateManager.closeOverlay();
    return true;
  }
  if (g.type != GestureType::Tap) return true;

  DisplayConfig config = displayPower.getConfig();
  switch (tileAt(g.startX, g.startY)) {
    case TileAutoDim:
      config.idleDimEnabled = !config.idleDimEnabled;
      displayPower.saveConfig(config);
      break;
    case TileNight:
      config.nightEnabled = !config.nightEnabled;
      displayPower.saveConfig(config);
      break;
    case TileRefresh:
      dataStore.requestRefresh();
      TimeUtil::syncer.syncNtpTime();
      refreshFeedbackUntil = millis() + 3000;
      break;
    case TilePortal: {
      String ip = WiFi.localIP().toString();
      qrRequest.title = "Web portal";
      qrRequest.subtitle = "http://" + ip;
      qrRequest.url = "http://" + ip + "/";
      qrRequest.hint =
          "Scan to set stops, display and weather. Your phone must be on the "
          "same WiFi.";
      appStateManager.openOverlay(AppState::StopQr);
      return true;
    }
    default:
      return true;
  }
  renderTiles();
  anim::presentRows(TILE_Y, TILE_H);
  return true;
}
