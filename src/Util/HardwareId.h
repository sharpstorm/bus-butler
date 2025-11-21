#pragma once

#include <memory>

#include "Arduino.h"

using namespace std;

class HardwareId {
 public:
  static unique_ptr<byte[]> getHardwareIdHash();
};
