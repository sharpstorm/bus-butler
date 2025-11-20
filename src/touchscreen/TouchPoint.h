#pragma once

#include <stdint.h>

class TouchPoint {
 public:
  uint8_t id;
  uint16_t x;
  uint16_t y;
  uint8_t size;

  TouchPoint(uint8_t id, uint16_t x, uint16_t y, uint16_t size);
  bool operator==(TouchPoint point);
  bool operator!=(TouchPoint point);
};
