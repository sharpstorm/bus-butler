#pragma once

#include "../IAppState.h"

class WeatherPageState : public IAppState {
 public:
  void enter() override;
  void loop() override;
  void exit() override {}
  bool isPage() override { return true; }
  bool allowsControls() override { return true; }

 private:
  uint32_t lastDataVersion = 0;
  uint32_t lastRenderMs = 0;
  String town;
  String region;
  void render();
  void renderNow(int16_t x, int16_t y, int16_t w, int16_t h);
  void renderToday(int16_t x, int16_t y, int16_t w, int16_t h);
  void renderOutlook(int16_t x, int16_t y, int16_t w, int16_t h);
};
