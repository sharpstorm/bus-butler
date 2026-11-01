#pragma once

#include <Arduino.h>

#include "Notification.h"

// Toast position: bottom centre, floating over every page
#define TOAST_X 32
#define TOAST_W (480 - 2 * TOAST_X)
#define TOAST_H 56
#define TOAST_Y (272 - TOAST_H - 10)
// A finger landing this close to the toast counts as touching it
#define TOAST_TOUCH_MARGIN 6
// Pause between one toast leaving and the next arriving
#define TOAST_GAP_MS 350

/*
  Runs on the UI loop and turns the store into what's on screen:
    - opens the full-screen overlay for the top full notification, switches
      it when a more urgent one arrives, closes it when none is left;
    - shows toasts one at a time over any page (via the anim overlay layer)
      and times them out;
    - opens a banner's details when it is tapped on the Home page.
  Banners themselves are drawn by the Home page (see HomeState).
*/
class NotifyUi {
 public:
  void loop();

  // Touch routing for toasts (main.cpp): a press that starts on the toast
  // belongs to it, and any tap or swipe there dismisses it.
  bool toastContains(int16_t x, int16_t y);
  void dismissToastByUser();

  // From the overlay
  void dismissFull(const String& id);
  void closeDetail();

  // From the Home page banner
  void openDetail(const String& bannerId);

  // Called by the anim overlay to paint the toast
  static void drawToast();

 private:
  uint32_t seenVersion = 0;
  uint32_t lastCheckMs = 0;
  uint32_t lastExpireMs = 0;

  bool toastVisible = false;
  Notification toast;
  uint32_t toastShownMs = 0;
  uint32_t toastGapUntil = 0;
  int16_t toastBarW = -1;
  uint32_t toastRefreshMs = 0;

  bool canShow();
  void reconcileFull();
  void reconcileToast();
  void tickToast();
  void showToast(const Notification& n);
  void hideToast();
};

extern NotifyUi notifyUi;
