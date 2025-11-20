#include "GT911TouchScreen.h"

#include <Arduino.h>

GT911TouchScreen::GT911TouchScreen(TwoWire& w, int sda, int scl, uint8_t addr,
                                   int rst, int interrupt)
    : wire(&w),
      sda(sda),
      scl(scl),
      addr(addr),
      rst(rst),
      interrupt(interrupt) {}

bool GT911TouchScreen::start() {
  if (isStarted) {
    return true;
  }

  isStarted = true;
  pinMode(interrupt, INPUT);
  if (rst != -1) {
    pinMode(rst, OUTPUT);
    digitalWrite(rst, 0);
    delay(200);
    digitalWrite(rst, 1);
    delay(200);
  }
  wire->begin(sda, scl);
  wire->beginTransmission(addr);
  wire->endTransmission();

  if (rst == -1) {
    this->writeRegister(GT911_COMMAND, (uint8_t)0x02);  // software reset
    delay(200);
    return false;
  }
  return true;
}

void GT911TouchScreen::stop() { wire->end(); }

// Screen On/Off Docs:
// https://www.fortec-integrated.de/fileadmin/pdf/produkte/Touchcontroller/DDGroup/GT911_Datasheet.pdf
void GT911TouchScreen::touchOff() {
  pinMode(interrupt, OUTPUT);
  digitalWrite(interrupt, LOW);
  this->writeRegister(GT911_COMMAND, GT911_CMD_SCREEN_OFF);
  // Must be > 58ms before screen can turn on again
  delay(60);
}

void GT911TouchScreen::touchOn() {
  pinMode(interrupt, OUTPUT);
  digitalWrite(interrupt, HIGH);
  delay(10);
  digitalWrite(interrupt, LOW);
  pinMode(interrupt, INPUT);
}

bool GT911TouchScreen::read() {
  registerMetadata = this->readRegister(GT911_POINT_INFO);
  this->writeRegister(GT911_POINT_INFO, GT911_CMD_READ_STATUS);  // sync signal

  uint8_t touchCount = registerMetadata & GT911_NUM_TOUCH_POINT_MASK;
  if (touchCount != 0) {
    uint8_t bytesToRead = touchCount * 8;
    this->readRegister(GT911_POINT_1, registerData, bytesToRead);
  }

  return touchCount != 0 ? true : false;
}

uint8_t GT911TouchScreen::getPointCount() {
  return registerMetadata & GT911_NUM_TOUCH_POINT_MASK;
}

// 0 <= n <= 4
TouchPoint GT911TouchScreen::getPoint(uint8_t n) {
  if (n > 4) {
    return TouchPoint(0, 0, 0, 0);
  }

  TouchPoint t(0, 0, 0, 0);
  uint16_t point_reg[5] = GT911_POINTS_REGISTERS;
  uint16_t offset = point_reg[n] - GT911_POINT_1;

  t.id = registerData[offset];
  t.x = registerData[offset + 1] + (registerData[offset + 2] << 8);
  t.y = registerData[offset + 3] + (registerData[offset + 4] << 8);
  t.size = registerData[offset + 5] | (registerData[offset + 6] << 8);

  // Rotation Transformations
  if (isVertical) {
    uint16_t tmp = t.x;
    t.x = t.y;
    t.y = tmp;
  }
  return t;
}

void GT911TouchScreen::setVertical(bool v) { isVertical = v; }
bool GT911TouchScreen::getVertical() { return isVertical; }

int GT911TouchScreen::writeRegister(uint8_t reg, uint8_t val) {
  wire->beginTransmission(addr);
  wire->write(reg);
  wire->write(val);
  return (wire->endTransmission() == 0) ? 0 : -1;
}

int GT911TouchScreen::writeRegister(uint16_t reg, uint8_t val) {
  wire->beginTransmission(addr);
  wire->write(reg >> 8);
  wire->write(reg & 0xff);
  wire->write(val);
  return (wire->endTransmission() == 0) ? 0 : -1;
}

int GT911TouchScreen::writeRegister(uint16_t reg, uint16_t val) {
  wire->beginTransmission(addr);
  wire->write(reg >> 8);
  wire->write(reg & 0xff);
  wire->write(val >> 8);
  wire->write(val & 0xff);
  return (wire->endTransmission() == 0) ? 0 : -1;
}

int GT911TouchScreen::writeRegister(uint8_t reg, uint8_t* buf, uint8_t length) {
  wire->beginTransmission(addr);
  wire->write(reg);
  wire->write(buf, length);
  return (wire->endTransmission() == 0) ? 0 : -1;
}

int GT911TouchScreen::readRegister(uint8_t reg) {
  wire->beginTransmission(addr);
  wire->write(reg);
  if (wire->endTransmission() != 0) {
    return -1;
  }
  wire->requestFrom(addr, 1U);
  return wire->read();
}

int GT911TouchScreen::readRegister(uint16_t reg) {
  wire->beginTransmission(addr);
  wire->write((uint8_t)(reg >> 8));
  wire->write((uint8_t)(reg & 0xff));
  if (wire->endTransmission() != 0) {
    return -1;
  }
  wire->requestFrom(addr, 1U);
  return wire->read();
}

int GT911TouchScreen::readRegister(uint8_t reg, uint8_t* buf, uint8_t length) {
  wire->beginTransmission(addr);
  wire->write(reg);
  if (wire->endTransmission() != 0) {
    return -1;
  }
  wire->requestFrom(addr, length);
  return wire->readBytes(buf, length) == length ? 0 : -1;
}

int GT911TouchScreen::readRegister(uint16_t reg, uint8_t* buf, uint8_t length) {
  wire->beginTransmission(addr);
  wire->write(reg >> 8);
  wire->write(reg & 0xff);
  if (wire->endTransmission() != 0) {
    return -1;
  }
  wire->requestFrom(addr, length);
  return wire->readBytes(buf, length) == length ? 0 : -1;
}
