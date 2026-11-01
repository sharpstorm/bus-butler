#pragma once

#include <Arduino.h>

#include <vector>

#include "../../Notify/JpegImage.h"
#include "../../Notify/Notification.h"
#include "../IAppState.h"

// Picture area on the left of a full-screen notification with a picture
#define NOTIFY_PIC_W 362
#define NOTIFY_PIC_H 272
// A banner opened from the Home page closes after this long untouched
#define NOTIFY_DETAIL_TIMEOUT_MS (30 * 1000)

/*
  Full-screen notification overlay. NotifyUi decides what it shows and when
  it opens and closes; this draws it and handles its buttons.

  Two modes:
    full   - a pushed full-screen notification; closes on its timeout, on a
             dismiss request, or when the user taps Close / swipes.
    detail - a banner opened from the Home page, to read it all and use its
             actions. Closing it leaves the banner in place.
*/
class NotifyState : public IAppState {
 public:
  void enter() override;
  void loop() override;
  void exit() override;
  bool onGesture(const Gesture& gesture) override;

  // Sets what to show; redraws now if the overlay is open.
  void show(const Notification& n, bool detail);
  const String& currentId() { return current.id; }
  bool isDetail() { return detail; }
  bool isActive() { return active; }

 private:
  struct Button {
    int16_t x, y, w, h;
    int action;  // index into current.actions, or -1 for Close
  };

  Notification current;
  bool detail = false;
  bool active = false;
  uint32_t seenVersion = 0;
  uint32_t openedMs = 0;  // detail: last touch
  int16_t lastBarW = -1;
  vector<Button> buttons;

  DecodedImage picture;
  const JpegBlob* pictureFrom = nullptr;  // the blob `picture` was decoded from
  shared_ptr<JpegBlob> pictureHold;       // keeps it alive while compared
  String pictureError;

  void render();
  void renderWithPicture();
  void renderCard();
  void renderPicture();
  void renderButton(const Button& b);
  // A label pill (badge, "Updating"); align right puts its right edge at x.
  void renderPill(int16_t x, int16_t y, const String& text, uint16_t dot,
                  bool alignRight);
  void renderBadge();
  void layoutColumn(int16_t x, int16_t w, int16_t bottom);
  void layoutRow(int16_t x, int16_t w, int16_t y, int16_t h);
  bool barGeometry(int16_t& x, int16_t& y, int16_t& w);
  void renderBar(bool present);
  float remaining();
  void applyBoost();
  void dismiss();
  bool sameLook(const Notification& n);
};

extern NotifyState notifyState;
