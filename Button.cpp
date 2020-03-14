#include "Button.h"

Button::Button(int p) {
  pin = p;
  pinMode(pin, INPUT_PULLUP);
  state = digitalRead(pin);
  stateTime = millis();
  cbClick = NULL;
  hide = false;
}

bool Button::pressed() {
  if (hide) return false;
  return state == LOW;
}

void Button::onClick(void (*handler)(int)) {
  cbClick = handler;
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
    if ((now - stateTime > 50) && (stateTime != 0)) {
      if (cbClick) cbClick(pin);
    }
  }
  state = newState;
  stateTime = now;
}

void Button::hideState(bool h) {
  hide = h;
}
