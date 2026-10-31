#pragma once

#include <Arduino.h>

#include "../IAppState.h"

// What the full-screen QR overlay shows. Set it, then
// appStateManager.openOverlay(AppState::StopQr).
struct QrRequest {
  String title;     // e.g. stop name
  String subtitle;  // e.g. "Stop 17179"
  String url;
  String hint;      // e.g. "Scan for live arrivals on bus.gov.sg"
};

extern QrRequest qrRequest;

#define QR_TIMEOUT_MS (30 * 1000)

class StopQrState : public IAppState {
 public:
  void enter() override;
  void loop() override;
  void exit() override {}
  bool onGesture(const Gesture& gesture) override;

 private:
  uint32_t enteredMs = 0;
  int lastSecondsLeft = -1;
  void renderCountdown(int secondsLeft, bool show);
};
