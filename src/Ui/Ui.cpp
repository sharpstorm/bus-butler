#include "Ui.h"

namespace ui {

static const GFXglyph* glyphFor(const GFXfont* font, char c) {
  uint8_t code = (uint8_t)c;
  if (code < font->first || code > font->last) return nullptr;
  return &font->glyph[code - font->first];
}

TextBounds measure(const GFXfont* font, const char* text) {
  TextBounds b{INT16_MAX, INT16_MAX, INT16_MIN, INT16_MIN, 0};
  int16_t cursor = 0;
  for (const char* p = text; *p; p++) {
    const GFXglyph* g = glyphFor(font, *p);
    if (g == nullptr) continue;
    if (g->width > 0 && g->height > 0) {
      b.minX = min<int16_t>(b.minX, cursor + g->xOffset);
      b.maxX = max<int16_t>(b.maxX, cursor + g->xOffset + g->width - 1);
      b.minY = min<int16_t>(b.minY, g->yOffset);
      b.maxY = max<int16_t>(b.maxY, g->yOffset + g->height - 1);
    }
    cursor += g->xAdvance;
  }
  b.advance = cursor;
  if (b.minX == INT16_MAX) {  // No ink (empty or all spaces)
    b.minX = b.maxX = b.minY = b.maxY = 0;
  }
  return b;
}

static void drawAt(const GFXfont* font, const String& text, int16_t x,
                   int16_t y, uint16_t color) {
  gfx->setFont(font);
  gfx->setTextSize(1);
  gfx->setTextWrap(false);
  gfx->setTextColor(color);
  gfx->setCursor(x, y);
  gfx->print(text);
}

void text(const GFXfont* font, const String& text, int16_t x, int16_t y,
          uint16_t color, Align align) {
  if (text.length() == 0) return;
  if (align == Left) {
    drawAt(font, text, x, y, color);
    return;
  }
  TextBounds b = measure(font, text.c_str());
  if (align == Right) {
    drawAt(font, text, x - b.maxX - 1, y, color);
  } else {
    drawAt(font, text, x - (b.minX + b.maxX + 1) / 2, y, color);
  }
}

void textMid(const GFXfont* font, const String& text, int16_t cx, int16_t cy,
             uint16_t color) {
  if (text.length() == 0) return;
  TextBounds b = measure(font, text.c_str());
  int16_t x = cx - (b.minX + b.maxX + 1) / 2;
  int16_t y = cy - (b.minY + b.maxY + 1) / 2;
  drawAt(font, text, x, y, color);
}

String fit(const GFXfont* font, const String& text, int16_t maxWidth) {
  if (width(font, text) <= maxWidth) return text;
  String s = text;
  while (s.length() > 0 && width(font, s + "..") > maxWidth) {
    s.remove(s.length() - 1);
  }
  s.trim();
  return s + "..";
}

vector<String> wrap(const GFXfont* font, const String& text, int16_t maxWidth,
                    int maxLines) {
  vector<String> lines;
  String current;
  int start = 0;
  int len = text.length();
  while (start < len) {
    int end = text.indexOf(' ', start);
    if (end < 0) end = len;
    String word = text.substring(start, end);
    start = end + 1;
    if (word.length() == 0) continue;

    String candidate = current.length() ? current + " " + word : word;
    if (width(font, candidate) <= maxWidth) {
      current = candidate;
      continue;
    }
    if (current.length()) {
      lines.push_back(current);
      if ((int)lines.size() == maxLines) {
        // Out of room: mark the last line as truncated
        lines.back() = fit(font, lines.back() + " " + word, maxWidth);
        if (!lines.back().endsWith("..")) lines.back() += "..";
        return lines;
      }
    }
    current = fit(font, word, maxWidth);  // A single over-long word
  }
  if (current.length() && (int)lines.size() < maxLines) {
    lines.push_back(current);
  }
  return lines;
}

void background(int16_t x, int16_t y, int16_t w, int16_t h) {
  gfx->fillRect(x, y, w, h, BG_DARK);
  for (int16_t gx = (x + 39) / 40 * 40; gx < x + w; gx += 40) {
    gfx->drawFastVLine(gx, y, h, GRID_LINE);
  }
  for (int16_t gy = (y + 39) / 40 * 40; gy < y + h; gy += 40) {
    gfx->drawFastHLine(x, gy, w, GRID_LINE);
  }
}

void background() { background(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); }

// Quarter circle centred at (cx, cy). quadrant: 0 TL, 1 TR, 2 BR, 3 BL
static void arc(int16_t cx, int16_t cy, int16_t r, int quadrant,
                uint16_t color) {
  int16_t f = 1 - r, ddx = 1, ddy = -2 * r, x = 0, y = r;
  auto plot = [&](int16_t px, int16_t py) {
    switch (quadrant) {
      case 0: gfx->drawPixel(cx - px, cy - py, color); break;
      case 1: gfx->drawPixel(cx + px, cy - py, color); break;
      case 2: gfx->drawPixel(cx + px, cy + py, color); break;
      default: gfx->drawPixel(cx - px, cy + py, color); break;
    }
  };
  plot(0, r);
  plot(r, 0);
  while (x < y) {
    if (f >= 0) {
      y--;
      ddy += 2;
      f += ddy;
    }
    x++;
    ddx += 2;
    f += ddx;
    plot(x, y);
    plot(y, x);
  }
}

void corners(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color,
             int16_t length, int16_t r) {
  int16_t x2 = x + w - 1, y2 = y + h - 1;
  int16_t run = length - r;
  // Top-left
  arc(x + r, y + r, r, 0, color);
  gfx->drawFastHLine(x + r, y, run, color);
  gfx->drawFastVLine(x, y + r, run, color);
  // Top-right
  arc(x2 - r, y + r, r, 1, color);
  gfx->drawFastHLine(x2 - r - run + 1, y, run, color);
  gfx->drawFastVLine(x2, y + r, run, color);
  // Bottom-right
  arc(x2 - r, y2 - r, r, 2, color);
  gfx->drawFastHLine(x2 - r - run + 1, y2, run, color);
  gfx->drawFastVLine(x2, y2 - r - run + 1, run, color);
  // Bottom-left
  arc(x + r, y2 - r, r, 3, color);
  gfx->drawFastHLine(x + r, y2, run, color);
  gfx->drawFastVLine(x, y2 - r - run + 1, run, color);
}

void section(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t corner) {
  gfx->fillRoundRect(x, y, w, h, 8, BG_CARD);
  corners(x, y, w, h, corner);
}

void sectionWithHeader(int16_t x, int16_t y, int16_t w, int16_t h,
                       int16_t headerH, uint16_t corner) {
  gfx->fillRoundRect(x, y, w, h, 8, BG_CARD);
  gfx->fillRoundRect(x, y, w, headerH + 8, 8, BG_HIGHLIGHT);
  gfx->fillRect(x, y + headerH, w, 8, BG_CARD);
  corners(x, y, w, h, corner);
}

int16_t dots(int16_t x, int16_t cy, int count, int current, uint16_t on,
             uint16_t off) {
  for (int i = 0; i < count; i++) {
    gfx->fillCircle(x + 2 + i * 8, cy, 2, i == current ? on : off);
  }
  return count > 0 ? count * 8 - 3 : 0;
}

String ago(uint32_t seconds) {
  if (seconds < 60) return String(seconds) + "s";
  if (seconds < 3600) return String(seconds / 60) + "m";
  if (seconds < 86400) return String(seconds / 3600) + "h";
  return String(seconds / 86400) + "d";
}

}  // namespace ui

namespace ui {

void pageHeader(const String& title, const String& subtitle, int pageIndex,
                int pageCount) {
  background();
  ui::text(&FigMed, title, 16, 24, NEON_CYAN);
  if (subtitle.length()) {
    int16_t x = 16 + width(&FigMed, title) + 10;
    ui::text(&SgSm, fit(&SgSm, subtitle, 400 - x), x, 23, GRAY_LIGHT);
  }
  int16_t dotsW = pageCount * 8 - 3;
  dots(SCREEN_WIDTH - 16 - dotsW, 19, pageCount, pageIndex, NEON_MAGENTA,
       GRAY_MID);
}

void row(int16_t x, int16_t w, int16_t y, const String& label,
         const String& value, uint16_t valueColor) {
  if (valueColor == 0) valueColor = TEXT_MAIN;
  ui::text(&SgSm, label, x, y, GRAY_LIGHT);
  int16_t labelW = width(&SgSm, label) + 10;
  ui::text(&SgMed, fit(&SgMed, value, w - labelW), x + w, y, valueColor,
           Right);
}

void dim(int16_t y, int16_t h) {
  uint16_t* fb = static_cast<Arduino_Canvas*>(gfx)->getFramebuffer();
  if (fb == nullptr) return;
  y = max<int16_t>(0, y);
  int16_t end = min<int16_t>(SCREEN_HEIGHT, y + h);
  for (int32_t i = (int32_t)y * SCREEN_WIDTH; i < (int32_t)end * SCREEN_WIDTH;
       i++) {
    // Halve each channel, then halve again: ~25% brightness
    uint16_t c = (fb[i] >> 1) & 0x7BEF;
    fb[i] = (c >> 1) & 0x7BEF;
  }
}

}  // namespace ui
