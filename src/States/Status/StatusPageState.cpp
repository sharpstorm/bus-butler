#include "StatusPageState.h"

#include <WiFi.h>

#include "../../Config/ConfigManager.h"
#include "../../Data/DataStore.h"
#include "../../Display/DisplayPower.h"
#include "../../Screen/Screen.h"
#include "../../Ui/Anim.h"
#include "../../Ui/Ui.h"
#include "../../Util/Time.h"
#include "../../Wifi/MDnsClient.h"
#include "../AppStateManager.h"

#define RENDER_INTERVAL_MS 2000
#define ROW_H 21

static String uptime() {
  uint32_t s = millis() / 1000;
  char buf[24];
  if (s >= 86400) {
    snprintf(buf, sizeof(buf), "%lud %luh", s / 86400, (s % 86400) / 3600);
  } else {
    snprintf(buf, sizeof(buf), "%luh %lum", s / 3600, (s % 3600) / 60);
  }
  return buf;
}

static String kb(uint32_t bytes) {
  if (bytes >= 1024 * 1024) return String(bytes / 1048576.0f, 1) + " MB";
  return String(bytes / 1024) + " KB";
}

static void sourceRow(int16_t x, int16_t w, int16_t y, const String& name,
                      const SourceStatus& s) {
  uint16_t color = GRAY_MID;
  String value = "Waiting";
  if (s.attempted) {
    if (s.ok) {
      color = STATUS_GREEN;
      value = ui::ago((millis() - s.lastSuccessMs) / 1000) + " ago";
    } else {
      color = STATUS_RED;
      value = s.lastError == -11 ? "No API key"
              : s.lastError == -10 ? "No stop"
              : s.lastSuccessMs > 0
                  ? "Failing, ok " +
                        ui::ago((millis() - s.lastSuccessMs) / 1000) + " ago"
                  : "Failing";
    }
  }
  gfx->fillCircle(x + 3, y - 4, 3, color);
  ui::row(x + 12, w - 12, y, name, value, s.attempted && !s.ok ? STATUS_RED : TEXT_MAIN);
}

void StatusPageState::enter() { render(); }

void StatusPageState::loop() {
  if (millis() - lastRenderMs > RENDER_INTERVAL_MS) render();
}

void StatusPageState::render() {
  lastRenderMs = millis();
  ui::pageHeader("Status", "Bus Butler v" FIRMWARE_VERSION, 3,
                 AppStateManager::PAGE_COUNT);

  // Device
  const int16_t lx = 8, lw = 228;
  ui::section(lx, 36, lw, 228, NEON_MAGENTA);
  ui::text(&SgSm, "DEVICE", lx + 12, 54, NEON_CYAN);
  int16_t x = lx + 12, w = lw - 24, y = 76;

  bool online = WiFi.isConnected();
  ui::row(x, w, y, "WiFi", online ? WiFi.SSID() : String("Disconnected"),
          online ? TEXT_MAIN : STATUS_RED);
  y += ROW_H;
  ui::row(x, w, y, "Signal", online ? String(WiFi.RSSI()) + " dBm" : "--");
  y += ROW_H;
  ui::row(x, w, y, "IP", online ? WiFi.localIP().toString() : "--");
  y += ROW_H;
  ui::row(x, w, y, "Portal", String(MDNS_HOSTNAME) + ".local", NEON_CYAN);
  y += ROW_H;
  ui::row(x, w, y, "Uptime", uptime());
  y += ROW_H;
  ui::row(x, w, y, "Clock", TimeUtil::isSynced() ? "Synced" : "Not synced",
          TimeUtil::isSynced() ? TEXT_MAIN : STATUS_RED);
  y += ROW_H;
  ui::row(x, w, y, "Free memory",
          kb(ESP.getFreeHeap()) + " + " + kb(ESP.getFreePsram()));
  y += ROW_H;
  // Last transition's frame rate and how long one full-screen send takes
  const anim::Stats& a = anim::stats();
  String animText = a.frames ? String(a.fps()) + " fps, " : String("");
  animText += a.flushUs ? String(a.flushUs / 1000.0f, 1) + " ms/frame"
                        : String("swipe to measure");
  ui::row(x, w, y, "Animation", animText);
  y += ROW_H;
  String screenState = String(displayPower.getAppliedLevel() * 100 / 255) + "%";
  if (displayPower.isIdleDimmed()) screenState += " (idle)";
  else if (displayPower.isNightActive()) screenState += " (night)";
  ui::row(x, w, y, "Screen", screenState);

  // Data sources
  const int16_t rx = 244, rw = 228;
  ui::section(rx, 36, rw, 228, NEON_CYAN);
  ui::text(&SgSm, "DATA", rx + 12, 54, NEON_CYAN);
  x = rx + 12;
  w = rw - 24;
  y = 76;

  dataStore.lock();
  sourceRow(x, w, y, "Bus arrivals", dataStore.busStatus);
  y += ROW_H;
  sourceRow(x, w, y, "MRT alerts", dataStore.trainStatus);
  y += ROW_H;
  sourceRow(x, w, y, "Forecast", dataStore.weatherStatus);
  y += ROW_H;
  sourceRow(x, w, y, "Readings", dataStore.extrasStatus);
  y += ROW_H;
  String activeStop = dataStore.getActiveStop();
  dataStore.unlock();

  y += 8;
  ui::row(x, w, y, "Saved stops",
          String(configManager.getBusStops().size()) + " (now " +
              (activeStop.length() ? activeStop : String("-")) + ")");
  y += ROW_H;
  ui::row(x, w, y, "LTA API key",
          configManager.getLtaDatamallApiKey().length() ? "Set" : "Missing",
          configManager.getLtaDatamallApiKey().length() ? TEXT_MAIN : STATUS_RED);
  y += ROW_H;
  String town = configManager.getWeatherTown();
  ui::row(x, w, y, "Weather town", town.length() ? town : String("Not set"),
          town.length() ? TEXT_MAIN : STATUS_YELLOW);

  anim::present();
}
