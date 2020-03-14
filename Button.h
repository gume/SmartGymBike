#pragma once
#include "Arduino.h"

class Button {
public:
  Button(int p);
  bool pressed();
  void onClick(void (*handler)(int));
  bool longPress(uint32_t durationms);
  void check();
  void hideState(bool); // Hide pressed state

private:
  bool state;
  bool hide;
  uint32_t stateTime;
  int pin;
  void (*cbClick)(int);
};
