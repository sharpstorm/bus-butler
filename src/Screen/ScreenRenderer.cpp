#include "ScreenRenderer.h"

void ScreenRenderer::drawCenterText(const char* text, int centerX,
                                    int centerY) {
  int16_t temp1, temp2;
  uint16_t width, height;
  gfx->getTextBounds(text, 0, 0, &temp1, &temp2, &width, &height);

  gfx->setCursor(centerX - (width / 2), centerY - (height / 2));
  gfx->println(text);
}

void ScreenRenderer::drawText(const char* text, int x, int y) {
  gfx->setCursor(x, y);
  gfx->println(text);
}
