#pragma once

#include <vector>

#include "Arduino.h"

using namespace std;

enum BusLoad {
  // SEA
  SeatsAvailable = 1,
  // SDA
  StandingAvailable = 2,
  // LSD
  LimitedStandingAvailable = 3,
};

enum BusType {
  // SD
  SingleDeck = 1,
  // DD
  DoubleDeck = 2,
  // BD
  Bendy = 3,
};

struct BusArrivalDetail {
  String originBusStop;
  String destinationBusStop;

  BusLoad load;
  BusType busType;
  bool isWheelchairAccessible;
};

struct BusArrivalServiceDetail {
  String serviceNo;
  String company;
  BusArrivalDetail nextBus;
  BusArrivalDetail nextBus2;
  BusArrivalDetail nextBus3;
};

struct BusArrivalApiResponse {
  String busStopCode;
  vector<BusArrivalServiceDetail> services;
  String timestamp;
};

String toReadableBusType(BusType type);
String toReadableBusLoad(BusLoad load);
