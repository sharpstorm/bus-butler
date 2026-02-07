#include "LayoutRenderer.h"

#include "../../Screen/Colors.h"
#include "../../Screen/Fonts/Icons.h"
#include "../../Screen/Fonts/RobotoMonoLight32.h"
#include "../../Screen/Fonts/RobotoMonoLightDigits40.h"
#include "../../Screen/Fonts/RobotoMonoMediumDigits30.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"
#include "HomescreenConstants.h"
#include "RendererUtil.h"

void LayoutRenderer::renderHeader() {
  gfx->fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, BG_PANEL);
  gfx->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, NEON_MAGENTA);
  gfx->drawFastHLine(0, HEADER_HEIGHT - 2, SCREEN_WIDTH, DIM_MAGENTA);

  // Draw left content box
  const int totalLBoxPadding = 2 * HEADER_L_BOX_PADDING;
  RendererUtil::drawCornerBrackets(HEADER_L_BOX_X, HEADER_L_BOX_Y,
                                   HEADER_L_BOX_WIDTH, HEADER_L_BOX_HEIGHT,
                                   NEON_CYAN, totalLBoxPadding * 2);
  gfx->fillRect(HEADER_L_BOX_X + HEADER_L_BOX_PADDING,
                HEADER_L_BOX_Y + HEADER_L_BOX_PADDING,
                HEADER_L_BOX_WIDTH - totalLBoxPadding,
                HEADER_L_BOX_HEIGHT - totalLBoxPadding, BG_CARD);
  gfx->drawRect(HEADER_L_BOX_X + HEADER_L_BOX_PADDING,
                HEADER_L_BOX_Y + HEADER_L_BOX_PADDING,
                HEADER_L_BOX_WIDTH - totalLBoxPadding,
                HEADER_L_BOX_HEIGHT - totalLBoxPadding, NEON_CYAN);

  RendererUtil::drawCornerBrackets(HEADER_R_BOX_X, HEADER_R_BOX_Y,
                                   HEADER_R_BOX_WIDTH, HEADER_R_BOX_HEIGHT,
                                   NEON_MAGENTA, 8);
  const int barHeight = HEADER_R_BOX_HEIGHT - (HEADER_R_BOX_PADDING * 2);
  const int barY = HEADER_R_BOX_Y + HEADER_R_BOX_PADDING;
  gfx->fillRect(HEADER_R_BOX_X + 12, barY, 4, barHeight, NEON_MAGENTA);
  gfx->fillRect(HEADER_R_BOX_X + 22, barY, 4, barHeight, DIM_MAGENTA);
  gfx->fillRect(HEADER_R_BOX_X + 32, barY, 4, barHeight, GRAY_MID);
}

void LayoutRenderer::renderFooter() {
  const int footerY = SCREEN_HEIGHT - FOOTER_HEIGHT;

  gfx->fillRect(0, footerY, SCREEN_WIDTH, FOOTER_HEIGHT, BG_PANEL);
  gfx->drawFastHLine(0, footerY, SCREEN_WIDTH, NEON_MAGENTA);
  gfx->drawFastHLine(0, footerY + 1, SCREEN_WIDTH, DIM_MAGENTA);

  const int btnY = footerY + 10;
  const int btnSize = 32;

  // Refresh button
  const int refreshX = SCREEN_WIDTH - 40;
  gfx->fillRect(refreshX, btnY, btnSize, btnSize, BG_CARD);
  gfx->drawRect(refreshX, btnY, btnSize, btnSize, NEON_CYAN);
  gfx->drawBitmap(refreshX + (btnSize - ICON_REFRESH_WIDTH) / 2,
                  btnY + (btnSize - ICON_REFRESH_HEIGHT) / 2, ICON_REFRESH,
                  ICON_REFRESH_WIDTH, ICON_REFRESH_HEIGHT, NEON_CYAN);

  // Settings button
  const int settingsX = SCREEN_WIDTH - 80;
  gfx->fillRect(settingsX, btnY, btnSize, btnSize, BG_CARD);
  gfx->drawRect(settingsX, btnY, btnSize, btnSize, GRAY_MID);
  gfx->drawBitmap(settingsX + (btnSize - ICON_COG_WIDTH) / 2,
                  btnY + (btnSize - ICON_COG_HEIGHT) / 2, ICON_COG,
                  ICON_COG_WIDTH, ICON_COG_HEIGHT, GRAY_LIGHT);
}

void LayoutRenderer::renderDateTime() {
  const int footerY = SCREEN_HEIGHT - FOOTER_HEIGHT;

  // Clear date/time area only
  gfx->fillRect(FOOTER_PADDING_X, footerY + 4, FOOTER_DATE_TIME_WIDTH,
                FOOTER_HEIGHT - 4, BG_PANEL);

  gfx->setFont(&Roboto_Mono_Light_32);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawText(TimeUtil::getTimeString(), 8, footerY + 24);

  gfx->setFont(NULL);
  gfx->setTextSize(1);
  gfx->setTextColor(NEON_CYAN);
  ScreenRenderer::drawText(TimeUtil::getDateString(), 8, footerY + 8);
}
