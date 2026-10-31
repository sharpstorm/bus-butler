#pragma once

#include <Arduino.h>

/*
  Screen presentation and transitions.

  Screens draw into the canvas framebuffer and call present() (whole screen)
  or presentRows() (a horizontal band, much cheaper) to show it.

  A transition is:
    anim::capture();          // remember what's on screen now
    ...draw the next screen...  // present() calls are held back meanwhile
    anim::slide(...);         // animate old -> new, then show new
  The two full-screen copies live in PSRAM (~255 KB each).
*/
namespace anim {

// Direction the content moves. Left = new screen enters from the right.
enum class Slide { Left, Right, Up, Down };

void present();
void presentRows(int16_t y, int16_t h);

void capture();
bool isCapturing();

// Full-screen push transition.
void slide(Slide direction, uint16_t durationMs = 260);

// Top panel (height panelH, already drawn at the top of the new frame)
// slides down over the old screen, which dims.
void panelDown(int16_t panelH, uint16_t durationMs = 220);
// Captured top panel slides up off the new screen.
void panelUp(int16_t panelH, uint16_t durationMs = 200);

// Vertical push inside a rectangle only (e.g. paging a list). up = the new
// content comes from below.
void slideRegion(int16_t x, int16_t y, int16_t w, int16_t h, bool up,
                 uint16_t durationMs = 220);

struct Stats {
  uint16_t frames = 0;     // frames in the last transition
  uint32_t durationMs = 0; // how long it took
  uint32_t flushUs = 0;    // last full-screen send time
  uint16_t fps() const {
    return durationMs ? (uint32_t)frames * 1000 / durationMs : 0;
  }
};
const Stats& stats();

}  // namespace anim
