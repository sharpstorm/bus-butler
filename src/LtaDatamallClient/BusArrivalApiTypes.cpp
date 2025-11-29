#include "BusArrivalApiTypes.h"

String toReadableBusType(BusType type) {
  switch (type) {
    case BusType::SingleDeck:
      return "Single Deck";
    case BusType::DoubleDeck:
      return "Double Deck";
    case BusType::Bendy:
      return "Bendy";
    default:
      return "Unknown";
  }
}

String toReadableBusLoad(BusLoad load) {
  switch (load) {
    case BusLoad::SeatsAvailable:
      return "Seats Available";
    case BusLoad::StandingAvailable:
      return "Standing Available";
    case BusLoad::LimitedStandingAvailable:
      return "Limited Standing Available";
    default:
      return "Unknown";
  }
}

BusLoad toBusLoadEnum(String load) {
  if (load == "SEA") {
    return BusLoad::SeatsAvailable;
  } else if (load == "SDA") {
    return BusLoad::StandingAvailable;
  } else {
    return BusLoad::LimitedStandingAvailable;
  }
}

BusType toBusTypeEnum(String type) {
  if (type == "BD") {
    return BusType::Bendy;
  } else if (type == "DD") {
    return BusType::DoubleDeck;
  } else {
    return BusType::SingleDeck;
  }
}
