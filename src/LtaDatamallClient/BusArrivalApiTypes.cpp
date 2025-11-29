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