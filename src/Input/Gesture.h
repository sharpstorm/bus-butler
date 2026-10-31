#pragma once

#include <Arduino.h>

// Distance (px) a finger must travel for a swipe, and the most it may move
// for a tap.
#define SWIPE_MIN_DISTANCE 40
#define TAP_MAX_DISTANCE 14
#define TAP_MAX_DURATION_MS 800
// A swipe down that starts this close to the top edge opens the controls.
#define TOP_EDGE_ZONE 18

enum class GestureType : uint8_t {
  Tap,
  SwipeLeft,   // finger moved right-to-left
  SwipeRight,  // finger moved left-to-right
  SwipeUp,
  SwipeDown,
};

struct Gesture {
  GestureType type;
  int16_t startX, startY;
  int16_t endX, endY;

  bool isHorizontal() const {
    return type == GestureType::SwipeLeft || type == GestureType::SwipeRight;
  }
  bool startsIn(int16_t x, int16_t y, int16_t w, int16_t h) const {
    return startX >= x && startX < x + w && startY >= y && startY < y + h;
  }
};

// Turns raw touch samples into taps and swipes.
class GestureTracker {
 public:
  void down(int16_t x, int16_t y, uint32_t nowMs) {
    sx = lx = x;
    sy = ly = y;
    startMs = nowMs;
  }
  void move(int16_t x, int16_t y) {
    lx = x;
    ly = y;
  }
  int16_t lastX() const { return lx; }
  int16_t lastY() const { return ly; }

  // Classify on release. Returns false if the movement was neither.
  bool release(uint32_t nowMs, Gesture& out) const {
    int16_t dx = lx - sx, dy = ly - sy;
    int16_t adx = abs(dx), ady = abs(dy);
    out.startX = sx;
    out.startY = sy;
    out.endX = lx;
    out.endY = ly;
    if (adx < TAP_MAX_DISTANCE && ady < TAP_MAX_DISTANCE &&
        nowMs - startMs < TAP_MAX_DURATION_MS) {
      out.type = GestureType::Tap;
      return true;
    }
    if (adx >= SWIPE_MIN_DISTANCE && adx * 10 > ady * 13) {
      out.type = dx < 0 ? GestureType::SwipeLeft : GestureType::SwipeRight;
      return true;
    }
    if (ady >= SWIPE_MIN_DISTANCE && ady * 10 > adx * 13) {
      out.type = dy < 0 ? GestureType::SwipeUp : GestureType::SwipeDown;
      return true;
    }
    return false;
  }

 private:
  int16_t sx = 0, sy = 0, lx = 0, ly = 0;
  uint32_t startMs = 0;
};
