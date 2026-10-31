#include "HomeState.h"

#include "../../Data/DataStore.h"
#include "../../Screen/Fonts/BusTypes.h"
#include "../../Screen/Screen.h"
#include "../../Ui/Anim.h"
#include "../../Ui/Ui.h"
#include "../../Ui/WeatherIcons.h"
#include "../../Util/Time.h"
#include "../AppStateManager.h"
#include "../StopQr/StopQrState.h"

#define RENDER_INTERVAL_MS 15000
#define PULSE_FRAME_MS 40      // ~25 fps for the arriving dot
#define PULSE_PERIOD_MS 1200
#define ARR_DOT_X (HOME_BUS_X + HOME_BUS_W - 12)
#define CONFIG_REFRESH_MS 60000

// Service numbers up to 3 characters use the large font; 4 (e.g. 243w)
// drop to the smaller one so they fit the 64px badge.
#define SVC_BADGE_W 64
#define SVC_BADGE_H 28

static const char* MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Orders services per the saved order, unlisted ones after in API order.
static vector<const BusArrivalServiceDetail*> sortServices(
    const BusArrivalApiResponse& resp) {
  vector<const BusArrivalServiceDetail*> result;
  BusServiceOrderConfig order = configManager.getBusServiceOrder();
  vector<bool> used(resp.services.size(), false);
  for (int i = 0; i < order.size && order.services != nullptr; i++) {
    for (size_t j = 0; j < resp.services.size(); j++) {
      if (!used[j] && resp.services[j].serviceNo == order.services[i]) {
        result.push_back(&resp.services[j]);
        used[j] = true;
        break;
      }
    }
  }
  for (size_t j = 0; j < resp.services.size(); j++) {
    if (!used[j]) result.push_back(&resp.services[j]);
  }
  return result;
}

static uint16_t loadColor(BusLoad load) {
  switch (load) {
    case BusLoad::SeatsAvailable:
      return STATUS_GREEN;
    case BusLoad::StandingAvailable:
      return STATUS_YELLOW;
    default:
      return STATUS_RED;
  }
}

static const char* busTypeGlyph(BusType type) {
  switch (type) {
    case BusType::Bendy:
      return BUS_TYPE_FONT_BENDY;
    case BusType::DoubleDeck:
      return BUS_TYPE_FONT_DOUBLE;
    default:
      return BUS_TYPE_FONT_SINGLE;
  }
}

static int minutesUntil(long timestamp, long now) {
  return (int)((timestamp - now) / 60);
}

// ---------------------------------------------------------------------------

void HomeState::enter() {
  syncStops();
  town = configManager.getWeatherTown();
  region = configManager.getWeatherRegion();
  lastConfigReadMs = millis();
  page = 0;
  render();
}

void HomeState::syncStops() {
  uint32_t revision = configManager.getStopsRevision();
  if (revision == stopsRevision && !stops.empty()) return;
  stopsRevision = revision;
  stops = configManager.getBusStops();
  stopIndex = configManager.getActiveBusStopIndex();
  if (stopIndex >= (int)stops.size()) stopIndex = 0;
  if (!stops.empty()) {
    dataStore.setActiveStop(stops[stopIndex].code);
  }
}

void HomeState::changeStop(int delta) {
  if (stops.size() < 2) return;
  stopIndex = (stopIndex + delta + stops.size()) % stops.size();
  configManager.saveActiveBusStopIndex(stopIndex);
  dataStore.setActiveStop(stops[stopIndex].code);
  page = 0;
  render();
}

void HomeState::loop() {
  uint32_t now = millis();
  if (now - lastConfigReadMs > CONFIG_REFRESH_MS) {
    lastConfigReadMs = now;
    town = configManager.getWeatherTown();
    region = configManager.getWeatherRegion();
  }

  bool dirty = false;
  if (configManager.getStopsRevision() != stopsRevision) {
    syncStops();
    dirty = true;
  }
  if (dataStore.getVersion() != lastDataVersion) dirty = true;
  if (now - lastRenderMs > RENDER_INTERVAL_MS) dirty = true;

  struct tm t;
  if (TimeUtil::localTime(t) && t.tm_min != lastMinute) dirty = true;

  if (dirty) {
    render();
  } else {
    pulseArrivingDots();
  }
}

// Small-area animation: only the dot's 13 rows are sent to the display.
void HomeState::pulseArrivingDots() {
  if (arrivingDots.empty() || millis() - lastPulseMs < PULSE_FRAME_MS) return;
  lastPulseMs = millis();
  float phase = (millis() % PULSE_PERIOD_MS) / (float)PULSE_PERIOD_MS;
  float wave = 0.5f - 0.5f * cosf(phase * 2 * PI);  // 0 -> 1 -> 0
  int16_t radius = 2 + (int16_t)lroundf(wave * 2.5f);
  for (int16_t cy : arrivingDots) {
    gfx->fillRect(ARR_DOT_X - 7, cy - 6, 14, 13, BG_CARD);
    gfx->fillCircle(ARR_DOT_X, cy, radius, STATUS_ARRIVING);
    anim::presentRows(cy - 6, 13);
  }
}

// Page through services with a vertical push inside the bus list.
void HomeState::changePage(int delta) {
  int next = page + delta;
  if (next < 0 || next >= pageCount) return;
  const int16_t regionY = HOME_TOP + HOME_BUS_HEADER_H + 1;
  const int16_t regionH = contentBottom() - 15 - regionY;
  anim::capture();
  page = next;
  render();
  anim::slideRegion(HOME_BUS_X + 1, regionY, HOME_BUS_W - 2, regionH,
                    delta > 0);
}

void HomeState::render() {
  lastRenderMs = millis();
  lastDataVersion = dataStore.getVersion();
  struct tm t;
  lastMinute = TimeUtil::localTime(t) ? t.tm_min : -1;

  arrivingDots.clear();
  dataStore.lock();
  alert = HomeAlert::None;
  if (dataStore.trainAlerts != nullptr && dataStore.trainAlerts->isDisrupted()) {
    alert = HomeAlert::Mrt;
  } else if (dataStore.weatherNow.heavyRainNearby ||
             isHeavyRainCondition(townNowcast())) {
    alert = HomeAlert::Rain;
  }
  alertShown = alert != HomeAlert::None;
  int16_t bottom = contentBottom();

  ui::background();
  renderHero(bottom, !alertShown);
  renderBuses(bottom);
  if (alert == HomeAlert::Mrt) renderAlert();
  if (alert == HomeAlert::Rain) renderRainAlert();
  dataStore.unlock();

  anim::present();
}

// ---------------------------------------------------------------------------
// Clock + weather panel

void HomeState::renderHero(int16_t bottom, bool showPeriods) {
  const int16_t x = HOME_HERO_X, w = HOME_HERO_W;
  ui::section(x, HOME_TOP, w, bottom - HOME_TOP, NEON_MAGENTA);

  struct tm t;
  if (TimeUtil::localTime(t)) {
    String date = TimeUtil::fullDayName(t.tm_wday) + " " + String(t.tm_mday) +
                  " " + MONTHS[t.tm_mon];
    ui::text(&FigMed, date, x + 12, 30, NEON_CYAN);

    int hour12 = t.tm_hour % 12 == 0 ? 12 : t.tm_hour % 12;
    char clock[6];
    snprintf(clock, sizeof(clock), "%d:%02d", hour12, t.tm_min);
    ui::text(&FigClock, clock, x + 10, 98, TEXT_MAIN);
    int16_t clockW = ui::width(&FigClock, clock);
    ui::text(&FigMed, t.tm_hour >= 12 ? "pm" : "am", x + 10 + clockW + 4, 98,
             NEON_MAGENTA);
  } else {
    ui::text(&FigMed, "Syncing time...", x + 12, 30, GRAY_LIGHT);
  }
  ui::text(&IconMd, ICON_SETTINGS, x + w - 30, 34, GRAY_LIGHT);

  // Now: 2-hour forecast for the town, nearest station temperature
  WeatherCondition now = WeatherCondition::Unknown;
  if (dataStore.twoHour != nullptr && town.length() > 0) {
    for (const auto& area : dataStore.twoHour->forecasts) {
      if (area.area.equalsIgnoreCase(town)) {
        now = area.condition;
        break;
      }
    }
  }
  const TwentyFourHourForecastResponse* day = dataStore.twentyFourHour.get();
  const WeatherNow& readings = dataStore.weatherNow;

  ui::text(&IconXl, ui::weatherIcon(now), x + 8, 170, ui::weatherColor(now));
  const SourceStatus& forecast = dataStore.weatherStatus;
  bool forecastStale = forecast.lastSuccessMs > 0 &&
                       millis() - forecast.lastSuccessMs > WEATHER_STALE_S * 1000UL;
  if (town.length() == 0) {
    ui::text(&FigSm, "SET TOWN", x + w - 12, 130, NEON_CYAN, ui::Right);
  } else if (forecastStale) {
    ui::text(&FigSm, "OLD", x + w - 12, 130, NEON_YELLOW, ui::Right);
  } else {
    ui::text(&FigSm, "2HR", x + w - 12, 130, NEON_CYAN, ui::Right);
  }

  String temp = "--";
  if (readings.hasAirTemp) {
    temp = String((int)lroundf(readings.airTemp));
  } else if (day != nullptr) {
    temp = String(day->temperature.high);
  }
  ui::text(&FigTemp, temp + DEG, x + 64, 154, NEON_ORANGE);

  String summary = ui::weatherLabel(now);
  if (day != nullptr) {
    summary += ", " + String(day->temperature.low) + "-" +
               String(day->temperature.high) + DEG;
  }
  ui::text(&FigSm, ui::fit(&FigSm, summary, w - 76), x + 66, 172, TEXT_MAIN);

  // One-line outlook: rain now, rain later today, or UV / dry
  String notice;
  uint16_t noticeColor = NEON_BLUE;
  const char* noticeIcon = ICON_RAIN;
  if (readings.hasRainfall && readings.rainfall > 0) {
    notice = "Raining nearby now";
  } else if (isRainyCondition(now)) {
    notice = "Rain in the next 2 hours";
  } else if (day != nullptr) {
    // periods[0] is the current period; look at the ones after it
    for (size_t i = 1; i < day->periods.size(); i++) {
      if (isRainyCondition(periodConditionForRegion(day->periods[i], region))) {
        String label = periodShortLabel(day->periods[i].timePeriod);
        String when = label == "Morn"    ? "this morning"
                      : label == "Aft"   ? "this afternoon"
                      : label == "Night" ? "tonight"
                                         : "after midnight";
        notice = "Showers likely " + when;
        break;
      }
    }
  }
  if (notice.length() == 0) {
    noticeIcon = ICON_UV;
    if (readings.uv >= 8) {
      notice = "UV very high (" + String(readings.uv) + ")";
      noticeColor = NEON_ORANGE;
    } else if (day != nullptr || readings.hasAirTemp) {
      notice = "No rain expected";
      noticeColor = GRAY_LIGHT;
    }
  }
  if (notice.length()) {
    ui::text(&IconSm, noticeIcon, x + 10, 196, noticeColor);
    ui::text(&FigSm, ui::fit(&FigSm, notice, w - 44), x + 30, 193,
             noticeColor);
  }

  // Upcoming periods strip (only when there's no MRT banner)
  if (showPeriods && day != nullptr && !day->periods.empty()) {
    const int16_t sy = 212, sh = 44;
    gfx->fillRoundRect(x + 8, sy, w - 16, sh, 6, BG_HIGHLIGHT);
    size_t count = min<size_t>(3, day->periods.size());
    int16_t colW = (w - 16) / 3;
    for (size_t i = 0; i < count; i++) {
      const PeriodForecast& p = day->periods[i];
      WeatherCondition c = periodConditionForRegion(p, region);
      int16_t cx = x + 8 + colW * i + 10;
      ui::text(&FigSm, periodShortLabel(p.timePeriod), cx, sy + 14, GRAY_LIGHT);
      ui::text(&IconMd, ui::weatherIcon(c), cx, sy + 40, ui::weatherColor(c));
    }
  }
}

// ---------------------------------------------------------------------------
// Bus panel

void HomeState::renderBuses(int16_t bottom) {
  const int16_t x = HOME_BUS_X, w = HOME_BUS_W;
  ui::sectionWithHeader(x, HOME_TOP, w, bottom - HOME_TOP, HOME_BUS_HEADER_H,
                        NEON_CYAN);

  const int rowsPerPage = alertShown ? 4 : 5;
  const int16_t rowH = alertShown ? 42 : 44;
  const int16_t firstRowY = HOME_TOP + HOME_BUS_HEADER_H + 6;

  // Services for the active stop
  vector<const BusArrivalServiceDetail*> services;
  bool haveData = false;
  String stopCode = stops.empty() ? String("") : stops[stopIndex].code;
  if (dataStore.busArrival != nullptr && dataStore.busArrivalStop == stopCode) {
    services = sortServices(*dataStore.busArrival);
    haveData = true;
  }
  // How old the arrivals are
  uint32_t ageS = 0;
  if (haveData && dataStore.busStatus.lastSuccessMs > 0) {
    ageS = (millis() - dataStore.busStatus.lastSuccessMs) / 1000;
  }
  const bool stale = haveData && ageS > BUS_STALE_S;
  const bool expired = haveData && ageS > BUS_EXPIRED_S;
  if (expired) services.clear();

  pageCount = max(1, ((int)services.size() + rowsPerPage - 1) / rowsPerPage);
  if (page >= pageCount) page = pageCount - 1;

  // Header: stop name, stop dots, page
  const int16_t headerBaseline = HOME_TOP + 17;
  int16_t right = x + w - 12;
  if (pageCount > 1) {
    String pageText = String(page + 1) + "/" + String(pageCount);
    ui::text(&SgSm, pageText, right, headerBaseline, NEON_MAGENTA, ui::Right);
    right -= ui::width(&SgSm, pageText) + 10;
  }
  if (stops.size() > 1) {
    int16_t dotsW = stops.size() * 8 - 3;
    ui::dots(right - dotsW, HOME_TOP + 13, stops.size(), stopIndex, NEON_CYAN,
             GRAY_MID);
    right -= dotsW + 10;
  }
  if (stale && !expired) {
    String age = ui::ago(ageS) + " old";
    ui::text(&SgSm, age, right, headerBaseline, NEON_YELLOW, ui::Right);
    right -= ui::width(&SgSm, age) + 10;
  }
  String name;
  if (stops.empty()) {
    name = "No bus stop set";
  } else if (stops[stopIndex].name.length() > 0) {
    name = stops[stopIndex].name;
  } else {
    name = "Stop " + stopCode;
  }
  ui::text(&SgSm, ui::fit(&SgSm, name, right - (x + 12)), x + 12,
           headerBaseline, NEON_CYAN);

  // Body
  const int16_t bodyCx = x + w / 2;
  const int16_t bodyCy = (firstRowY + bottom) / 2;
  if (stops.empty()) {
    ui::textMid(&FigMed, "Add a stop in the", bodyCx, bodyCy - 10, TEXT_MAIN);
    ui::textMid(&FigMed, "web portal", bodyCx, bodyCy + 10, TEXT_MAIN);
    return;
  }
  if (!haveData) {
    const SourceStatus& status = dataStore.busStatus;
    String message = "Loading arrivals...";
    if (status.attempted && !status.ok) {
      message = status.lastError == -11 ? "Set an LTA API key"
                                        : "Can't reach LTA";
    }
    ui::textMid(&FigMed, message, bodyCx, bodyCy, GRAY_LIGHT);
    return;
  }
  if (expired) {
    ui::textMid(&FigMed, "No update for " + ui::ago(ageS), bodyCx, bodyCy - 10,
                NEON_YELLOW);
    ui::textMid(&FigSm, "Check the Status page", bodyCx, bodyCy + 12,
                GRAY_LIGHT);
    return;
  }
  if (services.empty()) {
    ui::textMid(&FigMed, "No buses right now", bodyCx, bodyCy, GRAY_LIGHT);
    return;
  }

  long now = TimeUtil::getUnixTime();
  int start = page * rowsPerPage;
  int end = min<int>(start + rowsPerPage, services.size());
  for (int i = start; i < end; i++) {
    renderBusRow(firstRowY + (i - start) * rowH, *services[i], now, stale);
  }
}

void HomeState::renderBusRow(int16_t y, const BusArrivalServiceDetail& service,
                             long now, bool stale) {
  const int16_t x = HOME_BUS_X;

  // Service number in a neon rounded badge
  int16_t bx = x + 8, by = y + 5;
  gfx->fillRoundRect(bx, by, SVC_BADGE_W, SVC_BADGE_H, 5, BG_HIGHLIGHT);
  gfx->drawRoundRect(bx, by, SVC_BADGE_W, SVC_BADGE_H, 5, NEON_MAGENTA);
  const GFXfont* svcFont = service.serviceNo.length() <= 3 ? &SgSvcL : &SgSvc;
  if (ui::width(svcFont, service.serviceNo) > SVC_BADGE_W - 6) svcFont = &SgSvc;
  ui::textMid(svcFont, service.serviceNo, bx + SVC_BADGE_W / 2,
              by + SVC_BADGE_H / 2, TEXT_MAIN);

  const BusArrivalDetail& next = service.nextBus;
  if (next.arrivalTimestamp > 0) {
    ui::text(&BusTypesFont, busTypeGlyph(next.busType), x + 78, y + 6,
             NEON_BLUE);
  }

  // Following buses
  String following;
  if (service.nextBus2.arrivalTimestamp > 0) {
    following = String(max(0, minutesUntil(service.nextBus2.arrivalTimestamp, now)));
    if (service.nextBus3.arrivalTimestamp > 0) {
      following += ", " + String(max(0, minutesUntil(service.nextBus3.arrivalTimestamp, now)));
    }
  }
  if (following.length()) {
    ui::text(&SgSm, "Then", x + 118, y + 18, GRAY_LIGHT);
    ui::text(&SgMed, following, x + 118, y + 34, TEXT_SOFT);
  }

  // Next bus
  const int16_t right = x + HOME_BUS_W - 8;
  if (next.arrivalTimestamp <= 0) {
    ui::text(&SgBig, "--", right, y + 24, GRAY_MID, ui::Right);
    return;
  }
  int mins = minutesUntil(next.arrivalTimestamp, now);
  if (mins <= 0) {
    // Smaller "Arr" with the dot on the same line. Stale data: grey and
    // no pulse, since we don't know it's still true.
    uint16_t color = stale ? GRAY_LIGHT : STATUS_ARRIVING;
    gfx->fillCircle(ARR_DOT_X, y + 16, 3, color);
    if (!stale) arrivingDots.push_back(y + 16);
    ui::text(&SgMed, "Arr", right - 12, y + 22, color, ui::Right);
    return;
  }
  ui::text(&SgBig, mins > 99 ? String("99+") : String(mins), right, y + 24,
           stale ? GRAY_LIGHT : loadColor(next.load), ui::Right);
  ui::text(&SgSm, "min", right, y + 37, GRAY_LIGHT, ui::Right);
}

// ---------------------------------------------------------------------------
// MRT banner (only while there is a disruption)

void HomeState::renderAlert() {
  const TrainAlerts& alerts = *dataStore.trainAlerts;
  ui::section(8, HOME_ALERT_Y, 464, HOME_ALERT_H, NEON_MAGENTA);
  ui::text(&IconMd, ICON_WARNING, 20, HOME_ALERT_Y + 34, NEON_YELLOW);

  String title = "MRT disruption";
  String detail;
  if (!alerts.segments.empty()) {
    const TrainAlertSegment& seg = alerts.segments[0];
    title = trainLineShortName(seg.line) + " disruption";
    if (alerts.segments.size() > 1) {
      title += " +" + String(alerts.segments.size() - 1) + " more";
    }
    detail = stationRange(seg.stations);
    if (seg.direction.length() && !seg.direction.equalsIgnoreCase("both")) {
      detail += " (towards " + seg.direction + ")";
    }
    if (seg.freeBus.length()) detail += ", free buses";
  }
  if (!alerts.messages.empty()) {
    detail = alerts.messages[0];
  }

  const int16_t textW = 464 - 48 - 52;
  ui::text(&FigMed, ui::fit(&FigMed, title, textW), 48, HOME_ALERT_Y + 20,
           NEON_YELLOW);
  ui::text(&FigSm, ui::fit(&FigSm, detail, textW), 48, HOME_ALERT_Y + 37,
           GRAY_LIGHT);
  ui::text(&FigMed, "MRT", 460, HOME_ALERT_Y + 30, NEON_MAGENTA, ui::Right);
}

// Heavy rain banner: rain measured nearby right now, or heavy rain in the
// 2-hour forecast for the town.

WeatherCondition HomeState::townNowcast() {
  if (dataStore.twoHour == nullptr || town.length() == 0) {
    return WeatherCondition::Unknown;
  }
  for (const auto& area : dataStore.twoHour->forecasts) {
    if (area.area.equalsIgnoreCase(town)) return area.condition;
  }
  return WeatherCondition::Unknown;
}

void HomeState::renderRainAlert() {
  const WeatherNow& r = dataStore.weatherNow;
  ui::section(8, HOME_ALERT_Y, 464, HOME_ALERT_H, NEON_CYAN);
  ui::text(&IconMd, ICON_RAIN, 20, HOME_ALERT_Y + 34, NEON_BLUE);

  String title, detail;
  if (r.heavyRainNearby) {
    title = "Heavy rain nearby";
    detail = String((int)lroundf(r.nearbyRainRate)) + " mm/h at " +
             r.nearbyRainStation;
    if (r.nearbyRainKm >= 0.5f) {
      detail += ", " + String(r.nearbyRainKm, 1) + " km away";
    }
  } else {
    title = "Heavy rain expected";
    detail = "Next 2 hours in " + town + ": " +
             ui::weatherLabel(townNowcast());
  }

  const int16_t textW = 464 - 48 - 60;
  ui::text(&FigMed, ui::fit(&FigMed, title, textW), 48, HOME_ALERT_Y + 20,
           TEXT_MAIN);
  ui::text(&FigSm, ui::fit(&FigSm, detail, textW), 48, HOME_ALERT_Y + 37,
           GRAY_LIGHT);
  ui::text(&FigMed, "RAIN", 460, HOME_ALERT_Y + 30, NEON_BLUE, ui::Right);
}

// ---------------------------------------------------------------------------

bool HomeState::onGesture(const Gesture& g) {
  const int16_t bottom = contentBottom();
  bool inBusHeader = g.startsIn(HOME_BUS_X, HOME_TOP, HOME_BUS_W,
                                HOME_BUS_HEADER_H);
  bool inBusBody = g.startsIn(HOME_BUS_X, HOME_TOP + HOME_BUS_HEADER_H,
                              HOME_BUS_W, bottom - HOME_TOP - HOME_BUS_HEADER_H);

  switch (g.type) {
    case GestureType::SwipeLeft:
    case GestureType::SwipeRight:
      if (inBusHeader && stops.size() > 1) {
        changeStop(g.type == GestureType::SwipeLeft ? 1 : -1);
        return true;
      }
      return false;  // Page to page

    case GestureType::SwipeUp:
      if (inBusHeader || inBusBody) {
        changePage(1);
        return true;
      }
      return false;

    case GestureType::SwipeDown:
      if (inBusBody) {
        changePage(-1);
        return true;
      }
      return false;

    case GestureType::Tap: {
      if (g.startsIn(HOME_HERO_X + HOME_HERO_W - 44, HOME_TOP, 44, 40)) {
        appStateManager.openOverlay(AppState::Controls);
      } else if (alertShown && g.startsIn(8, HOME_ALERT_Y, 464, HOME_ALERT_H)) {
        appStateManager.goToPage(alert == HomeAlert::Mrt ? AppState::MrtPage
                                                         : AppState::WeatherPage);
      } else if ((inBusHeader || inBusBody) && !stops.empty()) {
        const BusStopConfig& stop = stops[stopIndex];
        qrRequest.title = stop.name.length() ? stop.name : "Bus stop";
        qrRequest.subtitle = "Stop " + stop.code;
        qrRequest.url = "https://bus.gov.sg/stops/" + stop.code;
        qrRequest.hint = "Scan for live arrivals and routes on bus.gov.sg";
        appStateManager.openOverlay(AppState::StopQr);
      } else if (g.startsIn(HOME_HERO_X, HOME_TOP, HOME_HERO_W, bottom)) {
        appStateManager.goToPage(AppState::WeatherPage);
      }
      return true;
    }
  }
  return false;
}
