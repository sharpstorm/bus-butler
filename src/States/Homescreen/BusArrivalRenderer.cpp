#include "BusArrivalRenderer.h"

#include "../../Screen/Colors.h"
#include "../../Screen/Fonts/BusTypes.h"
#include "../../Screen/Fonts/ErrorIcon.h"
#include "../../Screen/Fonts/Icons.h"
#include "../../Screen/Fonts/RobotoMonoLight32.h"
#include "../../Screen/Fonts/RobotoMonoLightDigits40.h"
#include "../../Screen/Fonts/RobotoMonoMediumDigits30.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"
#include "RendererUtil.h"

void BusArrivalRenderer::drawGlowBorder(int16_t x, int16_t y, int16_t w,
                                        int16_t h, uint16_t glowColor,
                                        uint16_t borderColor) {
  gfx->drawRect(x - 1, y - 1, w + 2, h + 2, glowColor);
  gfx->drawRect(x, y, w, h, borderColor);
  gfx->drawFastHLine(x + 1, y + 1, w - 2, borderColor);
}

void BusArrivalRenderer::drawCard(int16_t x, int16_t y,
                                  BusArrivalServiceDetail* data) {
  gfx->fillRect(x, y, BUS_ARRIVAL_CARD_WIDTH, BUS_ARRIVAL_CARD_HEIGHT, BG_CARD);
  drawGlowBorder(x, y, BUS_ARRIVAL_CARD_WIDTH, BUS_ARRIVAL_CARD_HEIGHT,
                 DIM_CYAN, NEON_CYAN);

  const int serviceBoxW = 80;
  gfx->fillRect(x + 2, y + 2, serviceBoxW, BUS_ARRIVAL_CARD_HEIGHT - 4,
                BG_HIGHLIGHT);
  gfx->drawRect(x + 2, y + 2, serviceBoxW, BUS_ARRIVAL_CARD_HEIGHT - 4,
                NEON_MAGENTA);

  String* serviceNo = &data->serviceNo;
  const bool isNumeric =
      all_of(serviceNo->begin(), serviceNo->end(), ::isdigit);

  gfx->setFont(isNumeric ? &Roboto_Mono_Light_Digits40 : &Roboto_Mono_Light_32);
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText(*serviceNo, x + 2 + serviceBoxW / 2,
                                 y + BUS_ARRIVAL_CARD_HEIGHT / 2);

  const long timeNow = TimeUtil::getUnixTime();

  drawArrivalTime(x + 90, y + 6, &data->nextBus, timeNow);
  drawArrivalTime(x + 158, y + 6, &data->nextBus2, timeNow);

  if (data->nextBus3.arrivalTimestamp > 0) {
    const int mins = (data->nextBus3.arrivalTimestamp - timeNow) / 60;
    if (mins >= 0 && mins < 99) {
      gfx->setFont(NULL);
      gfx->setTextSize(1);
      gfx->setTextColor(GRAY_LIGHT);
      ScreenRenderer::drawText(String(mins) + "m", x + 200,
                               y + BUS_ARRIVAL_CARD_HEIGHT - 16);
    }
  }

  gfx->setFont(&BusTypesFont);
  gfx->setTextSize(1);
  gfx->setTextColor(NEON_BLUE);

  if (data->nextBus.arrivalTimestamp > 0) {
    ScreenRenderer::drawText(getBusTypeIcon(data->nextBus.busType), x + 90,
                             y + 30);
  }
  if (data->nextBus2.arrivalTimestamp > 0) {
    ScreenRenderer::drawText(getBusTypeIcon(data->nextBus2.busType), x + 156,
                             y + 30);
  }
}

void BusArrivalRenderer::drawArrivalTime(int16_t x, int16_t y,
                                         BusArrivalDetail* arrival,
                                         long timeNow) {
  if (arrival->arrivalTimestamp <= 0) {
    gfx->setFont(NULL);
    gfx->setTextSize(2);
    gfx->setTextColor(GRAY_MID);
    ScreenRenderer::drawText("--", x, y + 8);
    gfx->setTextSize(1);
    return;
  }

  const int mins = (arrival->arrivalTimestamp - timeNow) / 60;
  const uint16_t color = getLoadColor(arrival->load);

  if (mins <= 0) {
    gfx->setFont(NULL);
    gfx->setTextSize(2);
    gfx->setTextColor(STATUS_ARRIVING);
    ScreenRenderer::drawText("ARR", x, y + 4);
    gfx->setTextSize(1);
    gfx->fillCircle(x + 42, y + 12, 4, STATUS_ARRIVING);
  } else {
    gfx->setFont(&Roboto_Mono_Medium_Digits30);
    gfx->setTextSize(1);
    gfx->setTextColor(color);

    String minsStr = String(mins);
    ScreenRenderer::drawText(minsStr, x, y);

    gfx->setFont(NULL);
    gfx->setTextSize(1);
    gfx->setTextColor(color);
    const int labelX = x + (mins >= 10 ? 40 : 24);
    ScreenRenderer::drawText("min", labelX, y + 12);
  }

  gfx->setTextSize(1);
}

void BusArrivalRenderer::renderBusService(uint8_t position,
                                          BusArrivalServiceDetail* data) {
  if (position >= 6) return;

  const int col = position % 2;
  const int row = position / 2;
  drawCard(cardX[col], cardY[row], data);
}

void BusArrivalRenderer::clearContent() {
  gfx->fillRect(0, CONTENT_TOP, SCREEN_WIDTH, CONTENT_HEIGHT, BG_DARK);

  for (int x = 0; x < SCREEN_WIDTH; x += 40) {
    gfx->drawFastVLine(x, CONTENT_TOP, CONTENT_HEIGHT, RGB565(12, 12, 18));
  }
  for (int y = CONTENT_TOP; y < SCREEN_HEIGHT - FOOTER_HEIGHT; y += 40) {
    gfx->drawFastHLine(0, y, SCREEN_WIDTH, RGB565(12, 12, 18));
  }
}

void BusArrivalRenderer::renderError(const char* message) {
  clearContent();

  const int centerY = CONTENT_TOP + CONTENT_HEIGHT / 2;

  const int iconX = (SCREEN_WIDTH - ERROR_ICON_WIDTH) / 2;
  const int iconY = centerY - ERROR_ICON_HEIGHT / 2 - 15;
  gfx->drawBitmap(iconX, iconY, ERROR_ICON_SAD_FACE, ERROR_ICON_WIDTH,
                  ERROR_ICON_HEIGHT, NEON_MAGENTA);

  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText(message, SCREEN_WIDTH / 2,
                                 iconY + ERROR_ICON_HEIGHT + 20);
  gfx->setTextSize(1);

  RendererUtil::drawCornerBrackets(SCREEN_WIDTH / 2 - 100, iconY - 10, 200,
                                   ERROR_ICON_HEIGHT + 60, NEON_MAGENTA, 15);

  finalize();
}

void BusArrivalRenderer::renderBusStopCode(String busStopCode) {
  gfx->setFont(NULL);
  gfx->setTextSize(1);
  gfx->setTextColor(NEON_CYAN);
  gfx->fillRect(HEADER_L_BOX_X + 1, HEADER_L_BOX_Y + 1, HEADER_L_BOX_WIDTH - 2,
                HEADER_L_BOX_HEIGHT - 2, BG_CARD);
  ScreenRenderer::drawCenterText(busStopCode,
                                 HEADER_L_BOX_X + (HEADER_L_BOX_WIDTH / 2),
                                 HEADER_L_BOX_Y + (HEADER_L_BOX_HEIGHT / 2));
}

void BusArrivalRenderer::finalize() { gfx->flush(); }

String BusArrivalRenderer::formatMins(long arrivalTimestamp, long timeNow) {
  int mins = (arrivalTimestamp - timeNow) / 60;
  if (mins <= 0) return "ARR";
  if (mins > 99) return "99+";
  return String(mins);
}

uint16_t BusArrivalRenderer::getLoadColor(BusLoad load) {
  switch (load) {
    case BusLoad::SeatsAvailable:
      return STATUS_GREEN;
    case BusLoad::StandingAvailable:
      return STATUS_YELLOW;
    default:
      return STATUS_RED;
  }
}

String BusArrivalRenderer::getBusTypeIcon(BusType type) {
  switch (type) {
    case BusType::Bendy:
      return BUS_TYPE_FONT_BENDY;
    case BusType::DoubleDeck:
      return BUS_TYPE_FONT_DOUBLE;
    default:
      return BUS_TYPE_FONT_SINGLE;
  }
}