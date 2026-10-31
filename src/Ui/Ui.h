#pragma once

#include <Arduino.h>

#include <vector>

#include "../Screen/Colors.h"
#include "../Screen/ScreenReference.h"
#include "Fonts/UiFonts.h"

using namespace std;


namespace ui {

enum Align { Left, Center, Right };

struct TextBounds {
  int16_t minX, minY, maxX, maxY;  // ink box relative to the cursor
  int16_t advance;                 // cursor advance
};

TextBounds measure(const GFXfont* font, const char* text);
inline int16_t width(const GFXfont* font, const String& text) {
  return measure(font, text.c_str()).advance;
}

// Draw at a GFX cursor (x, baseline y). Right aligns the ink's right edge.
void text(const GFXfont* font, const String& text, int16_t x, int16_t y,
          uint16_t color, Align align = Left);
// Draw so the ink box is centred on (cx, cy).
void textMid(const GFXfont* font, const String& text, int16_t cx, int16_t cy,
             uint16_t color);
// Truncate with ".." so the text fits within maxWidth.
String fit(const GFXfont* font, const String& text, int16_t maxWidth);
// Greedy word wrap to lines that fit maxWidth (at most maxLines; the last
// line is truncated with "..").
vector<String> wrap(const GFXfont* font, const String& text, int16_t maxWidth,
                    int maxLines);

// Page background: BG_DARK with the faint 40px grid.
void background();
void background(int16_t x, int16_t y, int16_t w, int16_t h);

// Rounded corner marks (no full outline), as on the current screens.
void corners(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color,
             int16_t length = 14, int16_t radius = 6);

// A section: tinted rounded panel with corner marks.
void section(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t corner);

// A section with a lighter header band of height headerH.
void sectionWithHeader(int16_t x, int16_t y, int16_t w, int16_t h,
                       int16_t headerH, uint16_t corner);

// Small row of dots, current one highlighted. Returns total width.
int16_t dots(int16_t x, int16_t cy, int count, int current, uint16_t on,
             uint16_t off);

// Seconds -> "12s", "4m", "3h", "2d"
String ago(uint32_t seconds);

// Background + title row for the swipeable pages: title, a dimmer subtitle
// after it, and page dots on the right.
void pageHeader(const String& title, const String& subtitle, int pageIndex,
                int pageCount);

// Label + right-aligned value row used on the status page.
void row(int16_t x, int16_t w, int16_t y, const String& label,
         const String& value, uint16_t valueColor = 0);

// Darken a region of the framebuffer (for overlays).
void dim(int16_t y, int16_t h);

}  // namespace ui
