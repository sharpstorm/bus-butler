#pragma once

#include <Wire.h>

#include "GT911Constants.h"
#include "TouchPoint.h"

class GT911TouchScreen {
 public:
  GT911TouchScreen(TwoWire& w, int sda, int scl, uint8_t addr, int rst,
                   int interrupt);

  bool start();
  void stop();

  void touchOff();
  void touchOn();

  bool read();
  uint8_t getPointCount();
  // 0 <= n <= 4
  TouchPoint getPoint(uint8_t n);

  void setVertical(bool isVertical);
  bool getVertical();

 private:
  bool isStarted = false;
  TwoWire* wire;
  int sda = -1;
  int scl = -1;
  int rst = -1;
  int interrupt = -1;
  uint8_t addr = 0xFF;
  bool isVertical = 0;

  // Data from Touch Chip, 0x814E
  uint8_t registerMetadata = 0;

  // 0x814F to 0x8175
  uint8_t registerData[39] = {0};

  int writeRegister(uint8_t reg, uint8_t val);
  int writeRegister(uint16_t reg, uint8_t val);
  int writeRegister(uint16_t reg, uint16_t val);
  int writeRegister(uint8_t reg, uint8_t* buf, uint8_t length);

  int readRegister(uint8_t reg);
  int readRegister(uint16_t reg);
  int readRegister(uint8_t reg, uint8_t* buf, uint8_t length);
  int readRegister(uint16_t reg, uint8_t* buf, uint8_t length);
};
