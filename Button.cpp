#include "Button.h"

Button::Button(int p) {
  pin = p;
  state = false;
  stateTime = millis();
  click = false;
  pinMode(pin, INPUT_PULLUP);
}

bool Button::pressed() {
  return state == LOW;
}

bool Button::clicked() {
  bool rc = click;
  click = false;
  return rc;
}

bool Button::longPress(uint32_t durationms) {
  if (state == LOW) { // Button pressed
    return millis() > stateTime + durationms;
  }
  return false;
}

void Button::check() {
  bool newState = digitalRead(pin);
  if (state == newState) return;

  uint32_t now = millis();
  if (newState == HIGH) {  // Button released
    if (now - stateTime > 50) click = true;
  }
  state = newState;
  stateTime = now;
}
