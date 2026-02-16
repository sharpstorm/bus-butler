#pragma once

class ITouchHandler {
 public:
  virtual void handleTouch(int x, int y);
  virtual void handleTouchStart(int x, int y) {}
  virtual void handleTouchEnd(int x, int y) {}
};
