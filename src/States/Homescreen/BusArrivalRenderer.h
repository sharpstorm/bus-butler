#pragma once

#include "../../LtaDatamallClient/BusArrivalApiTypes.h"
#include "../../NeaWeatherApi/NeaWeatherApiTypes.h"
#include "../../Screen/ScreenReference.h"
#include "Arduino.h"
#include "HomescreenConstants.h"

// Card grid - 2 columns, 3 rows (centered layout)
#define BUS_ARRIVAL_LAYOUT_MARGIN_X 8
#define BUS_ARRIVAL_LAYOUT_MARGIN_Y 3
#define BUS_ARRIVAL_CARD_WIDTH 228
#define BUS_ARRIVAL_CARD_HEIGHT 58
#define BUS_ARRIVAL_CARD_SPACING_X 8
#define BUS_ARRIVAL_CARD_SPACING_Y 3

// Columns
#define BUS_ARRIVAL_RENDER_COL_1_X BUS_ARRIVAL_LAYOUT_MARGIN_X
#define BUS_ARRIVAL_RENDER_COL_2_X                        \
  (BUS_ARRIVAL_LAYOUT_MARGIN_X + BUS_ARRIVAL_CARD_WIDTH + \
   BUS_ARRIVAL_CARD_SPACING_X)

// Rows
#define BUS_ARRIVAL_RENDER_ROW_1_Y (CONTENT_TOP + BUS_ARRIVAL_LAYOUT_MARGIN_Y)
#define BUS_ARRIVAL_RENDER_ROW_2_Y                                       \
  (CONTENT_TOP + BUS_ARRIVAL_LAYOUT_MARGIN_Y + BUS_ARRIVAL_CARD_HEIGHT + \
   BUS_ARRIVAL_CARD_SPACING_Y)
#define BUS_ARRIVAL_RENDER_ROW_3_Y                                             \
  (CONTENT_TOP + BUS_ARRIVAL_LAYOUT_MARGIN_Y + (2 * BUS_ARRIVAL_CARD_HEIGHT) + \
   (2 * BUS_ARRIVAL_CARD_SPACING_Y))

class BusArrivalRenderer {
 private:
  uint16_t cardX[2]{BUS_ARRIVAL_RENDER_COL_1_X, BUS_ARRIVAL_RENDER_COL_2_X};
  uint16_t cardY[4]{BUS_ARRIVAL_RENDER_ROW_1_Y, BUS_ARRIVAL_RENDER_ROW_2_Y,
                    BUS_ARRIVAL_RENDER_ROW_3_Y};

  void drawCard(int16_t x, int16_t y, BusArrivalServiceDetail* data);
  void drawGlowBorder(int16_t x, int16_t y, int16_t w, int16_t h,
                      uint16_t glowColor, uint16_t borderColor);
  void drawArrivalTime(int16_t x, int16_t y, BusArrivalDetail* arrival,
                       long timeNow);

  String formatMins(long arrivalTimestamp, long timeNow);
  uint16_t getLoadColor(BusLoad load);
  String getBusTypeIcon(BusType type);

 public:
  void renderBusService(uint8_t position, BusArrivalServiceDetail* data);
  void renderError(const char* message);
  void renderBusStopCode(String busStopCode);
  void renderPageIndicator(uint8_t currentPage, uint8_t totalPages);
  void clearContent();
  void finalize();
};