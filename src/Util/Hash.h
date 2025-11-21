#pragma once

#include <memory>

#include "Arduino.h"
#include "mbedtls/sha256.h"

using namespace std;

class Hash {
 public:
  static unique_ptr<byte[]> sha256(byte* payload);
};
