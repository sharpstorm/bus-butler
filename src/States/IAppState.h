#pragma once

#include "ITouchHandler.h"

class IAppState : public ITouchHandler {
 public:
  virtual void enter();
  virtual void loop();
  virtual void exit();
};
