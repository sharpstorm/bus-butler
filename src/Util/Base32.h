#pragma once

#include <memory>

#include "Arduino.h"
#include "stdint.h"

using namespace std;

struct Base32Result {
  int resultSize;
  unique_ptr<byte[]> result;
};

class Base32 {
 public:
  static Base32Result toBase32(byte*, long);
  static int fromBase32(byte*, long, byte*&);
};
