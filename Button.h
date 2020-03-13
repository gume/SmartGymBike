#pragma once
#include "Arduino.h"

class Button {
public:
  Button(int p);
  bool pressed();
  bool clicked();
  bool longPress(uint32_t durationms);
  void check();

private:
  bool state;
  uint32_t stateTime;
  int pin;
  bool click;
};
