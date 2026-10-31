#pragma once

#include "../IAppState.h"

#define FIRMWARE_VERSION "2.0.0"

class StatusPageState : public IAppState {
 public:
  void enter() override;
  void loop() override;
  void exit() override {}
  bool isPage() override { return true; }
  bool allowsControls() override { return true; }

 private:
  uint32_t lastRenderMs = 0;
  void render();
};
