#include "WeatherPageState.h"

#include "../../Config/ConfigManager.h"
#include "../../Data/DataStore.h"
#include "../../Screen/Screen.h"
#include "../../Ui/Anim.h"
#include "../../Ui/Ui.h"
#include "../../Ui/WeatherIcons.h"
#include "../AppStateManager.h"

#define RENDER_INTERVAL_MS 30000

static const char* psiBand(int psi, uint16_t& color) {
  if (psi <= 50) { color = STATUS_GREEN; return "Good"; }
  if (psi <= 100) { color = STATUS_YELLOW; return "Moderate"; }
  if (psi <= 200) { color = NEON_ORANGE; return "Unhealthy"; }
  if (psi <= 300) { color = STATUS_RED; return "V.unhealthy"; }
  color = NEON_PURPLE;
  return "Hazardous";
}

static const char* uvBand(int uv, uint16_t& color) {
  if (uv <= 2) { color = STATUS_GREEN; return "Low"; }
  if (uv <= 5) { color = STATUS_YELLOW; return "Moderate"; }
  if (uv <= 7) { color = NEON_ORANGE; return "High"; }
  if (uv <= 10) { color = STATUS_RED; return "Very high"; }
  color = NEON_PURPLE;
  return "Extreme";
}

// One small stat tile: icon, label, value, optional coloured band
static void statTile(int16_t x, int16_t y, int16_t w, const char* icon,
                     const String& label, const String& value,
                     const String& band, uint16_t bandColor) {
  gfx->fillRoundRect(x, y, w, 50, 6, BG_HIGHLIGHT);
  ui::text(&IconSm, icon, x + 8, y + 20, GRAY_LIGHT);
  ui::text(&SgSm, label, x + 28, y + 16, GRAY_LIGHT);
  ui::text(&SgMed, value, x + 8, y + 40, TEXT_MAIN);
  if (band.length()) {
    int16_t bx = x + 8 + ui::width(&SgMed, value) + 5;
    ui::text(&SgSm, ui::fit(&SgSm, band, x + w - 4 - bx), bx, y + 40,
             bandColor);
  }
}

void WeatherPageState::enter() {
  town = configManager.getWeatherTown();
  region = configManager.getWeatherRegion();
  if (region.length() == 0) region = "central";
  render();
}

void WeatherPageState::loop() {
  if (dataStore.getVersion() != lastDataVersion ||
      millis() - lastRenderMs > RENDER_INTERVAL_MS) {
    render();
  }
}

void WeatherPageState::render() {
  lastRenderMs = millis();
  lastDataVersion = dataStore.getVersion();

  String subtitle = town.length() ? town : String("Set a town in the portal");
  dataStore.lock();
  if (dataStore.weatherStatus.ok) {
    subtitle += "  -  updated " +
                ui::ago((millis() - dataStore.weatherStatus.lastSuccessMs) / 1000) +
                " ago";
  }
  ui::pageHeader("Weather", subtitle, 1, AppStateManager::PAGE_COUNT);

  renderNow(8, 36, 200, 228);
  renderToday(216, 36, 256, 116);
  renderOutlook(216, 160, 256, 104);
  dataStore.unlock();

  anim::present();
}

void WeatherPageState::renderNow(int16_t x, int16_t y, int16_t w, int16_t h) {
  ui::section(x, y, w, h, NEON_MAGENTA);
  ui::text(&SgSm, "NEXT 2 HOURS", x + 12, y + 18, NEON_CYAN);

  WeatherCondition now = WeatherCondition::Unknown;
  if (dataStore.twoHour != nullptr) {
    for (const auto& area : dataStore.twoHour->forecasts) {
      if (area.area.equalsIgnoreCase(town)) {
        now = area.condition;
        break;
      }
    }
  }
  const WeatherNow& r = dataStore.weatherNow;

  ui::text(&IconXl, ui::weatherIcon(now), x + 10, y + 76,
           ui::weatherColor(now));
  String temp = r.hasAirTemp ? String(r.airTemp, 1) : String("--");
  ui::text(&FigTemp, temp + DEG, x + 70, y + 60, NEON_ORANGE);
  ui::text(&FigSm, ui::fit(&FigSm, ui::weatherLabel(now), w - 82), x + 72,
           y + 78, TEXT_MAIN);
  if (r.airTempStation.length()) {
    ui::text(&SgSm, ui::fit(&SgSm, r.airTempStation, w - 24), x + 12, y + 98,
             GRAY_MID);
  }

  // 2 x 2 stat tiles
  const int16_t tileW = (w - 30) / 2;
  const int16_t tx1 = x + 10, tx2 = x + 20 + tileW;
  const int16_t ty1 = y + 110, ty2 = y + 168;

  statTile(tx1, ty1, tileW, ICON_DROP, "Humidity",
           r.hasHumidity ? String((int)lroundf(r.humidity)) + "%" : "--", "",
           0);
  statTile(tx2, ty1, tileW, ICON_RAIN, "Rain 5m",
           r.hasRainfall ? String(r.rainfall, 1) + "mm" : "--", "", 0);

  uint16_t bandColor = GRAY_LIGHT;
  String band = r.psi >= 0 ? psiBand(r.psi, bandColor) : "";
  statTile(tx1, ty2, tileW, ICON_MASK, "PSI 24h",
           r.psi >= 0 ? String(r.psi) : "--", band, bandColor);

  band = r.uv >= 0 ? uvBand(r.uv, bandColor) : "";
  statTile(tx2, ty2, tileW, ICON_UV, "UV now", r.uv >= 0 ? String(r.uv) : "--",
           band, bandColor);
}

void WeatherPageState::renderToday(int16_t x, int16_t y, int16_t w,
                                   int16_t h) {
  ui::section(x, y, w, h, NEON_CYAN);
  ui::text(&SgSm, "TODAY", x + 12, y + 18, NEON_CYAN);

  const TwentyFourHourForecastResponse* day = dataStore.twentyFourHour.get();
  if (day == nullptr) {
    ui::textMid(&FigSm, "Loading forecast...", x + w / 2, y + h / 2,
                GRAY_LIGHT);
    return;
  }

  String ranges = String(day->temperature.low) + "-" +
                  String(day->temperature.high) + DEG + "   " +
                  String(day->humidity.low) + "-" + String(day->humidity.high) +
                  "%";
  ui::text(&SgSm, ranges, x + w - 12, y + 18, NEON_ORANGE, ui::Right);

  size_t count = min<size_t>(4, day->periods.size());
  int16_t colW = (w - 16) / 4;
  for (size_t i = 0; i < count; i++) {
    const PeriodForecast& p = day->periods[i];
    WeatherCondition c = periodConditionForRegion(p, region);
    int16_t cx = x + 8 + colW * i + colW / 2;
    ui::text(&SgSm, periodShortLabel(p.timePeriod), cx, y + 40, GRAY_LIGHT,
             ui::Center);
    ui::text(&IconMd, ui::weatherIcon(c), cx - 11, y + 68,
             ui::weatherColor(c));
    ui::text(&SgSm, ui::fit(&SgSm, weatherConditionToShortString(c), colW - 4),
             cx, y + 84, TEXT_MAIN, ui::Center);
  }

  if (day->generalForecast.length()) {
    ui::text(&FigSm, ui::fit(&FigSm, day->generalForecast, w - 24), x + 12,
             y + h - 10, TEXT_SOFT);
  }
}

void WeatherPageState::renderOutlook(int16_t x, int16_t y, int16_t w,
                                     int16_t h) {
  ui::section(x, y, w, h, NEON_CYAN);
  ui::text(&SgSm, "NEXT 4 DAYS", x + 12, y + 18, NEON_CYAN);

  const FourDayOutlookResponse* outlook = dataStore.fourDay.get();
  if (outlook == nullptr || outlook->days.empty()) {
    ui::textMid(&FigSm, "Loading outlook...", x + w / 2, y + h / 2,
                GRAY_LIGHT);
    return;
  }

  size_t count = min<size_t>(4, outlook->days.size());
  int16_t colW = (w - 16) / 4;
  for (size_t i = 0; i < count; i++) {
    const DayOutlook& d = outlook->days[i];
    int16_t cx = x + 8 + colW * i + colW / 2;
    ui::text(&SgSm, d.day.substring(0, 3), cx, y + 38, GRAY_LIGHT, ui::Center);
    ui::text(&IconMd, ui::weatherIcon(d.condition), cx - 11, y + 66,
             ui::weatherColor(d.condition));
    ui::text(&SgSm,
             String(d.temperature.low) + "-" + String(d.temperature.high) + DEG,
             cx, y + 86, TEXT_MAIN, ui::Center);
  }
}
