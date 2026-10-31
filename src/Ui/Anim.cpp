#include "Anim.h"

#include <esp_heap_caps.h>

#include "../Screen/ScreenReference.h"

namespace anim {

static const int16_t W = SCREEN_WIDTH;
static const int16_t H = SCREEN_HEIGHT;
static const size_t FRAME_BYTES = (size_t)W * H * sizeof(uint16_t);

static uint16_t* fromBuf = nullptr;
static uint16_t* toBuf = nullptr;
static bool capturing = false;
static uint32_t captureMs = 0;
#define CAPTURE_TIMEOUT_MS 1000
static Stats lastStats;

static uint16_t* framebuffer() {
  return static_cast<Arduino_Canvas*>(gfx)->getFramebuffer();
}

static bool ensureBuffers() {
  if (fromBuf == nullptr) {
    fromBuf = (uint16_t*)heap_caps_malloc(FRAME_BYTES, MALLOC_CAP_SPIRAM);
  }
  if (toBuf == nullptr) {
    toBuf = (uint16_t*)heap_caps_malloc(FRAME_BYTES, MALLOC_CAP_SPIRAM);
  }
  return fromBuf != nullptr && toBuf != nullptr && framebuffer() != nullptr;
}

static void flushAll() {
  uint32_t start = micros();
  gfx->flush();
  lastStats.flushUs = micros() - start;
}

static void flushRows(int16_t y, int16_t h) {
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (y + h > H) h = H - y;
  if (h <= 0) return;
  g->draw16bitRGBBitmap(0, y, framebuffer() + (size_t)y * W, W, h);
}

// A capture that never got its transition must not freeze the screen.
static bool holding() {
  if (capturing && millis() - captureMs > CAPTURE_TIMEOUT_MS) {
    capturing = false;
  }
  return capturing;
}

void present() {
  if (!holding()) flushAll();
}

void presentRows(int16_t y, int16_t h) {
  if (!holding()) flushRows(y, h);
}

void capture() {
  if (!ensureBuffers()) return;  // No PSRAM: transitions just cut
  memcpy(fromBuf, framebuffer(), FRAME_BYTES);
  capturing = true;
  captureMs = millis();
}

bool isCapturing() { return capturing; }

const Stats& stats() { return lastStats; }

static float easeOutCubic(float t) {
  float u = 1 - t;
  return 1 - u * u * u;
}

// Runs frames until durationMs has passed. compose(e) builds the frame for
// eased progress e in [0,1] and returns the band of rows to send (or h = H
// for the whole screen).
template <typename Compose>
static void run(uint16_t durationMs, Compose compose) {
  uint32_t start = millis();
  uint16_t frames = 0;
  while (true) {
    float t = min(1.0f, (millis() - start) / (float)durationMs);
    int16_t y = 0, h = H;
    compose(easeOutCubic(t), y, h);
    if (h >= H) flushAll();
    else flushRows(y, h);
    frames++;
    if (t >= 1.0f) break;
  }
  lastStats.frames = frames;
  lastStats.durationMs = millis() - start;
}

// Ends a transition by showing the new frame exactly.
static void finish() {
  memcpy(framebuffer(), toBuf, FRAME_BYTES);
  flushAll();
}

// Halve each channel twice (~25%), same as ui::dim
static void dimRows(uint16_t* buf, int16_t y0, int16_t y1) {
  for (size_t i = (size_t)y0 * W; i < (size_t)y1 * W; i++) {
    uint16_t c = (buf[i] >> 1) & 0x7BEF;
    buf[i] = (c >> 1) & 0x7BEF;
  }
}

void slide(Slide direction, uint16_t durationMs) {
  if (!capturing) {
    flushAll();
    return;
  }
  capturing = false;
  uint16_t* fb = framebuffer();
  memcpy(toBuf, fb, FRAME_BYTES);

  run(durationMs, [&](float e, int16_t&, int16_t&) {
    switch (direction) {
      case Slide::Left: {
        int16_t o = e * W;
        for (int16_t r = 0; r < H; r++) {
          uint16_t* dst = fb + (size_t)r * W;
          memcpy(dst, fromBuf + (size_t)r * W + o, (W - o) * 2);
          memcpy(dst + W - o, toBuf + (size_t)r * W, o * 2);
        }
        break;
      }
      case Slide::Right: {
        int16_t o = e * W;
        for (int16_t r = 0; r < H; r++) {
          uint16_t* dst = fb + (size_t)r * W;
          memcpy(dst, toBuf + (size_t)r * W + (W - o), o * 2);
          memcpy(dst + o, fromBuf + (size_t)r * W, (W - o) * 2);
        }
        break;
      }
      case Slide::Up: {
        int16_t o = e * H;
        memcpy(fb, fromBuf + (size_t)o * W, (size_t)(H - o) * W * 2);
        memcpy(fb + (size_t)(H - o) * W, toBuf, (size_t)o * W * 2);
        break;
      }
      case Slide::Down: {
        int16_t o = e * H;
        memcpy(fb, toBuf + (size_t)(H - o) * W, (size_t)o * W * 2);
        memcpy(fb + (size_t)o * W, fromBuf, (size_t)(H - o) * W * 2);
        break;
      }
    }
  });
  finish();
}

void panelDown(int16_t panelH, uint16_t durationMs) {
  if (!capturing) {
    flushAll();
    return;
  }
  capturing = false;
  uint16_t* fb = framebuffer();
  memcpy(toBuf, fb, FRAME_BYTES);

  // The old screen dims straight away; only the panel moves.
  dimRows(fromBuf, 0, H);
  memcpy(fb, fromBuf, FRAME_BYTES);
  flushAll();

  run(durationMs, [&](float e, int16_t& y, int16_t& h) {
    int16_t o = e * panelH;
    memcpy(fb, toBuf + (size_t)(panelH - o) * W, (size_t)o * W * 2);
    memcpy(fb + (size_t)o * W, fromBuf + (size_t)o * W,
           (size_t)(panelH - o) * W * 2);
    y = 0;
    h = panelH;
  });
  finish();
}

void panelUp(int16_t panelH, uint16_t durationMs) {
  if (!capturing) {
    flushAll();
    return;
  }
  capturing = false;
  uint16_t* fb = framebuffer();
  memcpy(toBuf, fb, FRAME_BYTES);

  // Below the panel the new screen appears at once (it was dimmed before).
  memcpy(fb + (size_t)panelH * W, toBuf + (size_t)panelH * W,
         (size_t)(H - panelH) * W * 2);
  flushRows(panelH, H - panelH);

  run(durationMs, [&](float e, int16_t& y, int16_t& h) {
    int16_t o = e * panelH;
    memcpy(fb, fromBuf + (size_t)o * W, (size_t)(panelH - o) * W * 2);
    memcpy(fb + (size_t)(panelH - o) * W, toBuf + (size_t)(panelH - o) * W,
           (size_t)o * W * 2);
    y = 0;
    h = panelH;
  });
  finish();
}

void slideRegion(int16_t x, int16_t y, int16_t w, int16_t h, bool up,
                 uint16_t durationMs) {
  if (!capturing) {
    flushAll();
    return;
  }
  capturing = false;
  uint16_t* fb = framebuffer();
  memcpy(toBuf, fb, FRAME_BYTES);

  // Everything outside the region switches to the new frame immediately.
  memcpy(fb, toBuf, FRAME_BYTES);
  flushAll();

  run(durationMs, [&](float e, int16_t& by, int16_t& bh) {
    int16_t o = e * h;
    for (int16_t i = 0; i < h; i++) {
      // Which source row shows at region row i
      const uint16_t* src;
      if (up) {
        int16_t s = i + o;
        src = s < h ? fromBuf + (size_t)(y + s) * W
                    : toBuf + (size_t)(y + s - h) * W;
      } else {
        int16_t s = i - o;
        src = s >= 0 ? fromBuf + (size_t)(y + s) * W
                     : toBuf + (size_t)(y + s + h) * W;
      }
      memcpy(fb + (size_t)(y + i) * W + x, src + x, w * 2);
    }
    by = y;
    bh = h;
  });
  finish();
}

}  // namespace anim
