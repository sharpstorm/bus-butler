#pragma once

#include <Arduino.h>

#include <vector>

using namespace std;

// One affected stretch of a line, from LTA's TrainServiceAlerts API.
struct TrainAlertSegment {
  String line;         // e.g. "EWL"
  String direction;    // e.g. "Joo Koon", or "Both"
  String stations;     // comma separated station codes, e.g. "EW23,EW24"
  String freeBus;      // stations with free public bus, comma separated
  String freeShuttle;  // stations with free MRT shuttle, comma separated
};

struct TrainAlerts {
  // 1 = normal service, 2 = disrupted (as returned by LTA)
  int status = 1;
  vector<TrainAlertSegment> segments;
  vector<String> messages;

  bool isDisrupted() const { return status == 2 || !segments.empty(); }
};

// Human readable line name, e.g. "EWL" -> "East-West Line"
String trainLineName(const String& code);
// Short name for tight spaces, e.g. "EWL" -> "EW Line"
String trainLineShortName(const String& code);
// RGB565 line colour (official line colours)
uint16_t trainLineColor(const String& code);
// "EW23,EW24,EW25" -> "EW23 - EW25"
String stationRange(const String& stations);
