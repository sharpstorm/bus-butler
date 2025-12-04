#pragma once

#include "../../LtaDatamallClient/BusArrivalApiTypes.h"
#include "Arduino.h"

#define BUS_ARRIVAL_RENDER_COL_1_X 10
#define BUS_ARRIVAL_RENDER_COL_2_X 250

#define BUS_ARRIVAL_RENDER_ROW_1_Y 10
#define BUS_ARRIVAL_RENDER_ROW_2_Y 75
#define BUS_ARRIVAL_RENDER_ROW_3_Y 140
#define BUS_ARRIVAL_RENDER_ROW_4_Y 205

class BusArrivalRenderer {
 private:
  uint8_t renderGridX[2]{BUS_ARRIVAL_RENDER_COL_1_X,
                         BUS_ARRIVAL_RENDER_COL_2_X};
  uint8_t renderGridY[4]{BUS_ARRIVAL_RENDER_ROW_1_Y, BUS_ARRIVAL_RENDER_ROW_2_Y,
                         BUS_ARRIVAL_RENDER_ROW_3_Y,
                         BUS_ARRIVAL_RENDER_ROW_4_Y};

  String toReadableMins(long arrivalTimestamp, long timeNow);
  uint16_t resolveLoadColor(BusLoad load);
  String resolveBusTypeIcon(BusType type);

 public:
  BusArrivalRenderer* renderPosition(uint8_t position,
                                     BusArrivalServiceDetail* arrivalData);
  BusArrivalRenderer* renderToolbar(const String& busStopCode);
  void finalize();
};
