#include "TouchPoint.h"

TouchPoint::TouchPoint(uint8_t id, uint16_t x, uint16_t y, uint16_t size)
    : id(id), x(x), y(y), size(size) {}

bool TouchPoint::operator==(TouchPoint point) {
  return ((point.x == x) && (point.y == y) && (point.size == size));
}

bool TouchPoint::operator!=(TouchPoint point) {
  return ((point.x != x) || (point.y != y) || (point.size != size));
}
