#pragma once

#include <Arduino.h>

// An RGB565 picture in PSRAM, in the canvas's pixel format.
struct DecodedImage {
  uint16_t* pixels = nullptr;
  int16_t w = 0, h = 0;

  DecodedImage() = default;
  DecodedImage(const DecodedImage&) = delete;
  DecodedImage& operator=(const DecodedImage&) = delete;
  ~DecodedImage() { reset(); }
  void reset() {
    free(pixels);
    pixels = nullptr;
    w = h = 0;
  }
  bool ok() const { return pixels != nullptr; }
};

/*
  Decodes a baseline JPEG to fit within maxW x maxH. The decoder can only
  shrink by 1/2, 1/4 or 1/8, so the largest of those that fits is used; a
  picture still too big at 1/8 is cropped around its centre. Progressive
  JPEGs are refused. Takes ~30-60 ms for a 362x272 picture.
*/
bool decodeJpeg(const uint8_t* data, size_t len, int16_t maxW, int16_t maxH,
                DecodedImage& out, String& error);
