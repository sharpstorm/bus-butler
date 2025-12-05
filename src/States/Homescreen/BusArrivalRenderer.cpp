#include "BusArrivalRenderer.h"

#include "../../Screen/Fonts/BusTypes.h"
#include "../../Screen/Fonts/Icons.h"
#include "../../Screen/Fonts/RobotoMonoLight32.h"
#include "../../Screen/Fonts/RobotoMonoLightDigits40.h"
#include "../../Screen/Fonts/RobotoMonoMediumDigits30.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"

BusArrivalRenderer* BusArrivalRenderer::renderPosition(
    uint8_t position, BusArrivalServiceDetail* arrivalData) {
  if (position >= 7) {
    return this;
  }

  int x = renderGridX[position % 2];
  int y = renderGridY[position / 2];
  long timeNow = TimeUtil::getUnixTime();

  gfx->fillRect(x, y, 235, 60, BLACK);

  // Service box
  gfx->drawRoundRect(x, y, 90, 50, 2, MAGENTA);

  // Service
  String* serviceNoCstr = &arrivalData->serviceNo;
  bool isNumeric =
      all_of(serviceNoCstr->begin(), serviceNoCstr->end(), ::isdigit);

  gfx->setFont(isNumeric ? &Roboto_Mono_Light_Digits40 : &Roboto_Mono_Light_32);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText(arrivalData->serviceNo, x + 40, y + 25);

  BusArrivalDetail* arrival1 = &arrivalData->nextBus;
  BusArrivalDetail* arrival2 = &arrivalData->nextBus2;
  bool hasArrival1 = arrival1->arrivalTimestamp > 0;
  bool hasArrival2 = arrival2->arrivalTimestamp > 0;

  // Arrival 1 and 2
  gfx->setFont(&Roboto_Mono_Medium_Digits30);
  if (hasArrival1) {
    gfx->setTextColor(resolveLoadColor(arrival1->load));
    ScreenRenderer::drawText(
        toReadableMins(arrival1->arrivalTimestamp, timeNow), x + 100, y + 27);
  }

  if (hasArrival2) {
    gfx->setTextColor(resolveLoadColor(arrival2->load));
    ScreenRenderer::drawText(
        toReadableMins(arrival2->arrivalTimestamp, timeNow), x + 155, y + 27);
  }

  // Bus type 1 and 2
  gfx->setFont(&BusTypesFont);
  gfx->setTextColor(CYAN);
  if (hasArrival1) {
    ScreenRenderer::drawText(resolveBusTypeIcon(arrival1->busType), x + 100,
                             y - 2);
  }
  if (hasArrival2) {
    ScreenRenderer::drawText(resolveBusTypeIcon(arrival2->busType), x + 155,
                             y - 2);
  }

  return this;
}

BusArrivalRenderer* BusArrivalRenderer::renderToolbar(
    const String& busStopCode) {
  gfx->setFont(NULL);
  gfx->setTextColor(WHITE);
  gfx->fillRect(SCREEN_WIDTH - 40, SCREEN_HEIGHT - 16, 40, 16,
                RGB565(60, 60, 60));
  ScreenRenderer::drawCenterText(busStopCode, SCREEN_WIDTH - 20,
                                 SCREEN_HEIGHT - 8);
  gfx->setFont(&IconFont);
  gfx->fillRect(SCREEN_WIDTH - 24, SCREEN_HEIGHT - 44, 20, 20,
                RGB565(40, 40, 40));
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText(ICON_REFRESH, SCREEN_WIDTH - 14,
                                 SCREEN_HEIGHT - 34);
  return this;
}

void BusArrivalRenderer::finalize() { gfx->flush(); }

String BusArrivalRenderer::toReadableMins(long arrivalTimestamp, long timeNow) {
  int32_t minsToArrival = (arrivalTimestamp - timeNow) / 60;
  if (minsToArrival <= 0) {
    // TODO: Make arriving nicer
    return "0";
  }
  return String(minsToArrival);
}

uint16_t BusArrivalRenderer::resolveLoadColor(BusLoad load) {
  switch (load) {
    case BusLoad::SeatsAvailable:
      return GREEN;
    case BusLoad::StandingAvailable:
      return YELLOW;
    default:
      return PALERED;
  }
}

String BusArrivalRenderer::resolveBusTypeIcon(BusType type) {
  switch (type) {
    case BusType::Bendy:
      return BUS_TYPE_FONT_BENDY;
    case BusType::DoubleDeck:
      return BUS_TYPE_FONT_DOUBLE;
    default:
      return BUS_TYPE_FONT_SINGLE;
  }
}
