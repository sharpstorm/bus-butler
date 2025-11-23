#include "HardwareId.h"

#include "./Hash.h"

unique_ptr<byte[]> HardwareId::getHardwareIdHash() {
  uint64_t espChipId = ESP.getEfuseMac();
  byte deviceId[] = {
      (byte)(espChipId & 0xFF),         (byte)((espChipId >> 8) & 0xFF),
      (byte)((espChipId >> 16) & 0xFF), (byte)((espChipId >> 24) & 0xFF),
      (byte)((espChipId >> 32) & 0xFF), (byte)((espChipId >> 40) & 0xFF),
      (byte)((espChipId >> 48) & 0xFF), (byte)((espChipId >> 56) & 0xFF),
      (byte)(espChipId & 0xFF),         (byte)((espChipId >> 8) & 0xFF),
      (byte)((espChipId >> 16) & 0xFF), (byte)((espChipId >> 24) & 0xFF),
      (byte)((espChipId >> 32) & 0xFF), (byte)((espChipId >> 40) & 0xFF),
      (byte)((espChipId >> 48) & 0xFF), (byte)((espChipId >> 56) & 0xFF),
      (byte)(espChipId & 0xFF),         (byte)((espChipId >> 8) & 0xFF),
      (byte)((espChipId >> 16) & 0xFF), (byte)((espChipId >> 24) & 0xFF),
      (byte)((espChipId >> 32) & 0xFF), (byte)((espChipId >> 40) & 0xFF),
      (byte)((espChipId >> 48) & 0xFF), (byte)((espChipId >> 56) & 0xFF),
      (byte)(espChipId & 0xFF),         (byte)((espChipId >> 8) & 0xFF),
      (byte)((espChipId >> 16) & 0xFF), (byte)((espChipId >> 24) & 0xFF),
      (byte)((espChipId >> 32) & 0xFF), (byte)((espChipId >> 40) & 0xFF),
      (byte)((espChipId >> 48) & 0xFF), (byte)((espChipId >> 56) & 0xFF)};

  unique_ptr<byte[]> hashSecret = Hash::sha256(deviceId);
  return move(hashSecret);
}
