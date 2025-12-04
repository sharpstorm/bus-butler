#include "ScreenRenderer.h"

void ScreenRenderer::drawCenterText(const char* text, int centerX,
                                    int centerY) {
  int16_t temp1, temp2;
  uint16_t width, height;
  gfx->getTextBounds(text, 0, 0, &temp1, &temp2, &width, &height);

  gfx->setCursor(centerX - (width / 2), centerY - (height / 2));
  gfx->println(text);
}

void ScreenRenderer::drawCenterText(const String& text, int centerX,
                                    int centerY) {
  drawCenterText(text.c_str(), centerX, centerY);
}

void ScreenRenderer::drawText(const char* text, int x, int y) {
  gfx->setCursor(x, y);
  gfx->println(text);
}

void ScreenRenderer::drawText(const String& text, int x, int y) {
  drawText(text.c_str(), x, y);
}

void ScreenRenderer::drawRightAlignText(const char* text, int x, int y) {
  int16_t temp1, temp2;
  uint16_t width, height;
  gfx->getTextBounds(text, 0, 0, &temp1, &temp2, &width, &height);

  gfx->setCursor(x - width, y - height);
  gfx->println(text);
}

void ScreenRenderer::drawRightAlignText(const String& text, int x, int y) {
  drawRightAlignText(text.c_str(), x, y);
}
