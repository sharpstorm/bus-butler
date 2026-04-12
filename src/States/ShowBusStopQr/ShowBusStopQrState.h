#pragma once

#include <memory>

#include "../IAppState.h"

using namespace std;

class ShowBusStopQrState : public IAppState {
 public:
  void enter();
  void loop();
  void exit();

  void handleTouch(int x, int y);

 private:
  int entryTimestamp;
};
