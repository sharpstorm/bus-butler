#pragma once

#include "../IAppState.h"

class MrtPageState : public IAppState {
 public:
  void enter() override;
  void loop() override;
  void exit() override {}
  bool onGesture(const Gesture& gesture) override;
  bool isPage() override { return true; }
  bool allowsControls() override { return true; }

 private:
  uint32_t lastDataVersion = 0;
  uint32_t lastRenderMs = 0;
  int scroll = 0;  // message lines scrolled past
  int maxScroll = 0;
  void render();
  void renderNormal();
  void renderDisrupted();
};
