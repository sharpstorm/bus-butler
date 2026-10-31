#pragma once

#include "../IAppState.h"

#define CONTROLS_H 186
#define SLIDER_X 52
#define SLIDER_Y 52
#define SLIDER_W 352
#define SLIDER_H 20
#define TILE_Y 94
#define TILE_H 78
#define TILE_W 108
#define TILE_GAP 8
#define TILE_X0 12

class ControlsState : public IAppState {
 public:
  void enter() override;
  void loop() override;
  void exit() override;
  void onTouchDown(int16_t x, int16_t y) override;
  void onTouchMove(int16_t x, int16_t y) override;
  void onTouchUp(int16_t x, int16_t y) override;
  bool onGesture(const Gesture& gesture) override;

 private:
  bool dragging = false;
  bool brightnessChanged = false;
  uint8_t brightness = 255;
  uint32_t refreshFeedbackUntil = 0;
  int lastMinute = -1;

  void render();
  void renderSlider();
  void renderTiles();
  void setFromX(int16_t x);
  int tileAt(int16_t x, int16_t y);
};
