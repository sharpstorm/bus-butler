#include "JpegImage.h"

#include <JPEGDEC.h>
#include <esp_heap_caps.h>

#include <new>

struct DecodeTarget {
  uint16_t* pixels;
  int16_t w, h;    // output size
  int16_t cx, cy;  // crop offset in the scaled picture
};

static int drawBlock(JPEGDRAW* draw) {
  DecodeTarget* t = static_cast<DecodeTarget*>(draw->pUser);
  int used = draw->iWidthUsed > 0 ? draw->iWidthUsed : draw->iWidth;
  for (int row = 0; row < draw->iHeight; row++) {
    int y = draw->y + row - t->cy;
    if (y < 0 || y >= t->h) continue;
    const uint16_t* src = draw->pPixels + (size_t)row * draw->iWidth;
    for (int col = 0; col < used; col++) {
      int x = draw->x + col - t->cx;
      if (x < 0 || x >= t->w) continue;
      t->pixels[(size_t)y * t->w + x] = src[col];
    }
  }
  return 1;  // keep going
}

bool decodeJpeg(const uint8_t* data, size_t len, int16_t maxW, int16_t maxH,
                DecodedImage& out, String& error) {
  out.reset();
  // The decoder keeps ~20 KB of state; keep it out of internal RAM.
  void* mem = heap_caps_malloc(sizeof(JPEGDEC), MALLOC_CAP_SPIRAM);
  if (mem == nullptr) {
    error = "out of memory";
    return false;
  }
  JPEGDEC* jpeg = new (mem) JPEGDEC();
  bool ok = false;
  DecodeTarget target = {};

  if (!jpeg->openRAM(const_cast<uint8_t*>(data), (int)len, drawBlock)) {
    error = "not a JPEG the device can read";
  } else if (jpeg->getJPEGType() == JPEG_MODE_PROGRESSIVE) {
    error = "progressive JPEGs aren't supported";
    jpeg->close();
  } else {
    int iw = jpeg->getWidth(), ih = jpeg->getHeight();
    int scale = 1, option = 0;
    const int options[] = {0, JPEG_SCALE_HALF, JPEG_SCALE_QUARTER,
                           JPEG_SCALE_EIGHTH};
    for (int i = 0; i < 4; i++) {
      scale = 1 << i;
      option = options[i];
      if ((iw + scale - 1) / scale <= maxW && (ih + scale - 1) / scale <= maxH) {
        break;
      }
    }
    int sw = (iw + scale - 1) / scale, sh = (ih + scale - 1) / scale;
    target.w = min(sw, (int)maxW);
    target.h = min(sh, (int)maxH);
    target.cx = (sw - target.w) / 2;
    target.cy = (sh - target.h) / 2;
    target.pixels = (uint16_t*)heap_caps_calloc(
        (size_t)target.w * target.h, 2, MALLOC_CAP_SPIRAM);
    if (target.pixels == nullptr || target.w <= 0 || target.h <= 0) {
      error = "out of memory";
      jpeg->close();
    } else {
      jpeg->setPixelType(RGB565_LITTLE_ENDIAN);
      jpeg->setUserPointer(&target);
      ok = jpeg->decode(0, 0, option) == 1;
      if (!ok) error = "decode failed (error " + String(jpeg->getLastError()) + ")";
      jpeg->close();
    }
  }
  jpeg->~JPEGDEC();
  heap_caps_free(mem);

  if (ok) {
    out.pixels = target.pixels;
    out.w = target.w;
    out.h = target.h;
  } else {
    free(target.pixels);
  }
  return ok;
}
