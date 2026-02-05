#include "RendererUtil.h"

void RendererUtil::drawCornerBrackets(int16_t x, int16_t y, int16_t w,
                                      int16_t h, uint16_t color, int16_t len) {
  gfx->drawFastHLine(x, y, len, color);
  gfx->drawFastVLine(x, y, len, color);
  gfx->drawFastHLine(x + w - len, y, len, color);
  gfx->drawFastVLine(x + w - 1, y, len, color);
  gfx->drawFastHLine(x, y + h - 1, len, color);
  gfx->drawFastVLine(x, y + h - len, len, color);
  gfx->drawFastHLine(x + w - len, y + h - 1, len, color);
  gfx->drawFastVLine(x + w - 1, y + h - len, len, color);
}
