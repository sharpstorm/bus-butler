#pragma once

#include "Arduino.h"

class TimeUtil {
 public:
  static long iso8601StringToUnixTime(String time);
};
