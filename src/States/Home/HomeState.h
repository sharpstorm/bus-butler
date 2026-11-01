#pragma once

#include <vector>

#include "../../Config/ConfigManager.h"
#include "../../LtaDatamallClient/BusArrivalApiTypes.h"
#include "../../NeaWeatherApi/NeaWeatherApiTypes.h"
#include "../../Notify/Notification.h"
#include "../IAppState.h"

using namespace std;

// Layout (see the A2 / A3 mockups)
#define HOME_HERO_X 8
#define HOME_HERO_W 216
#define HOME_BUS_X 232
#define HOME_BUS_W 240
#define HOME_TOP 8
#define HOME_BUS_HEADER_H 26
#define HOME_ALERT_Y 216
#define HOME_ALERT_H 48

// Bus data older than this is shown dimmed with its age; much older and the
// arrivals are hidden (the minutes would be misleading).
#define BUS_STALE_S 120
#define BUS_EXPIRED_S 900
// Forecast older than this shows "OLD" instead of "2HR"
#define WEATHER_STALE_S (30 * 60)

// The banner slot along the bottom, one at a time. Order: pushed banners of
// priority 2+ (high / urgent), MRT disruption, other pushed banners, rain.
enum class HomeAlert { None, Mrt, Rain, Notice };

class HomeState : public IAppState {
 public:
  void enter() override;
  void loop() override;
  void exit() override {}
  bool onGesture(const Gesture& gesture) override;
  bool isPage() override { return true; }
  bool allowsControls() override { return true; }

 private:
  vector<BusStopConfig> stops;
  uint32_t stopsRevision = 0;
  int stopIndex = 0;
  int page = 0;
  int pageCount = 1;
  bool alertShown = false;
  HomeAlert alert = HomeAlert::None;
  Notification notice;  // the pushed banner shown, when alert == Notice
  int noticeCount = 0;  // pushed banners in total
  uint32_t lastBannerVersion = 0;
  String town;
  String region;

  uint32_t lastDataVersion = 0;
  uint32_t lastRenderMs = 0;
  uint32_t lastConfigReadMs = 0;
  int lastMinute = -1;

  // "Arr" dots currently on screen (y of each dot centre), for the pulse
  vector<int16_t> arrivingDots;
  uint32_t lastPulseMs = 0;
  void pulseArrivingDots();
  void changePage(int delta);

  void syncStops();
  void changeStop(int delta);
  void render();
  void renderHero(int16_t bottom, bool showPeriods);
  void renderBuses(int16_t bottom);
  void renderBusRow(int16_t y, const BusArrivalServiceDetail& service,
                    long now, bool stale);
  void renderAlert();
  void renderRainAlert();
  void renderNoticeAlert();
  WeatherCondition townNowcast();
  int16_t contentBottom() { return alertShown ? HOME_TOP + 200 : 264; }
};
