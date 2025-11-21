#pragma once

class IAppState {
 public:
  virtual void enter();
  virtual void loop();
  virtual void exit();
};
