#pragma once

#include <Arduino.h>

#include "../Input/Gesture.h"

class IAppState {
 public:
  virtual void enter() = 0;
  virtual void loop() = 0;
  virtual void exit() = 0;

  // Raw touch, for things that track the finger (e.g. sliders).
  virtual void onTouchDown(int16_t x, int16_t y) {}
  virtual void onTouchMove(int16_t x, int16_t y) {}
  virtual void onTouchUp(int16_t x, int16_t y) {}

  // A completed tap or swipe. Return true if the state handled it; unhandled
  // left/right swipes on pages move to the neighbouring page.
  virtual bool onGesture(const Gesture& gesture) { return false; }

  // Part of the swipeable Home / Weather / MRT / Status carousel.
  virtual bool isPage() { return false; }
  // Pulling down from the top edge opens the controls panel.
  virtual bool allowsControls() { return false; }
};

enum AppState {
  Init = 0,
  Boot,
  Setup,
  Home,
  WeatherPage,
  MrtPage,
  StatusPage,
  StopQr,
  Controls,
  Notify,  // full-screen notification overlay
};

class IAppStateResolver {
 public:
  virtual IAppState* getStateRefById(AppState state);
};
