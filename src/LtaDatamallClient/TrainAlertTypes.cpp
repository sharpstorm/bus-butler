#include "TrainAlertTypes.h"

#include <Arduino_GFX_Library.h>

struct LineInfo {
  const char* code;
  const char* name;
  const char* shortName;
  uint16_t color;
};

static const LineInfo LINES[] = {
    {"NSL", "North-South Line", "NS Line", RGB565(214, 46, 18)},
    {"EWL", "East-West Line", "EW Line", RGB565(0, 150, 69)},
    {"CGL", "Changi Airport Branch", "CG Line", RGB565(0, 150, 69)},
    {"NEL", "North East Line", "NE Line", RGB565(153, 0, 170)},
    {"CCL", "Circle Line", "Circle Line", RGB565(250, 158, 13)},
    {"CEL", "Circle Line Extension", "Circle Line", RGB565(250, 158, 13)},
    {"DTL", "Downtown Line", "DT Line", RGB565(0, 94, 196)},
    {"TEL", "Thomson-East Coast Line", "TE Line", RGB565(157, 91, 37)},
    {"BPL", "Bukit Panjang LRT", "BP LRT", RGB565(116, 132, 116)},
    {"SLRT", "Sengkang LRT", "SK LRT", RGB565(116, 132, 116)},
    {"PLRT", "Punggol LRT", "PG LRT", RGB565(116, 132, 116)},
};

static const LineInfo* findLine(const String& code) {
  for (const auto& line : LINES) {
    if (code.equalsIgnoreCase(line.code)) return &line;
  }
  return nullptr;
}

String trainLineName(const String& code) {
  const LineInfo* line = findLine(code);
  return line ? String(line->name) : code;
}

String trainLineShortName(const String& code) {
  const LineInfo* line = findLine(code);
  return line ? String(line->shortName) : code;
}

uint16_t trainLineColor(const String& code) {
  const LineInfo* line = findLine(code);
  return line ? line->color : RGB565(128, 128, 128);
}

String stationRange(const String& stations) {
  String s = stations;
  s.trim();
  if (s.length() == 0) return "";
  int first = s.indexOf(',');
  if (first < 0) return s;
  int last = s.lastIndexOf(',');
  String a = s.substring(0, first);
  String b = s.substring(last + 1);
  a.trim();
  b.trim();
  return a + " - " + b;
}
