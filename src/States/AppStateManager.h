#pragma once

#include "IAppState.h"

class AppStateManager {
 private:
  IAppStateResolver* stateResolver;

  AppState activeState = AppState::Init;
  IAppState* activeStateRef = nullptr;
  AppState returnState = AppState::Home;  // where overlays go back to

 public:
  // Meant for internal use during bootstrapping. No need to call under any
  // circumstance
  void registerAppStateResolver(IAppStateResolver* resolver);
  IAppState* getActiveState();
  AppState getActiveStateId();
  void changeState(AppState state);

  // Routes a finished gesture: controls pull-down, then the active state,
  // then page-to-page swipes.
  void dispatchGesture(const Gesture& gesture);

  // Open an overlay (controls, QR) that returns to the current state.
  // Both animate.
  void openOverlay(AppState overlay);
  void closeOverlay();

  // Go to one of the swipeable pages with a horizontal slide.
  void goToPage(AppState page);

  // Redraw the current screen from scratch (e.g. after a palette change).
  void redraw();

  // Swipeable pages, in order.
  static const AppState PAGES[];
  static const int PAGE_COUNT;
  int pageIndex(AppState state);
};

extern AppStateManager appStateManager;
