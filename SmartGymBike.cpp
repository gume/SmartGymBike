#include "SmartGymBike.h"

SmartGymBike::SmartGymBike() {

  // Setup button pad
  pinMode(BTN_G1, OUTPUT);
  pinMode(BTN_G2, OUTPUT);
  digitalWrite(BTN_G1, LOW);
  digitalWrite(BTN_G2, LOW);

  pinMode(BTN_Q, INPUT_PULLUP);
  pinMode(BTN_P, INPUT_PULLUP);
  pinMode(BTN_S, INPUT_PULLUP);
  pinMode(BTN_E, INPUT_PULLUP);
  pinMode(BTN_D, INPUT_PULLUP);
  pinMode(BTN_U, INPUT_PULLUP);

  // Setup level controller
  pinMode(PIN_MP, OUTPUT);
  pinMode(PIN_MN, OUTPUT);
  digitalWrite(PIN_MP, LOW);
  digitalWrite(PIN_MN, LOW);

  // Setup level sensor
  pinMode(PIN_POSVCC, OUTPUT);
  pinMode(PIN_POSGND, OUTPUT);
  pinMode(PIN_POS, INPUT);
  digitalWrite(PIN_POSVCC, HIGH);
  digitalWrite(PIN_POSGND, LOW);

  targetLevel = analogRead(PIN_POS);
  targetReached = true;

  // Setup cadence sensor
  pinMode(PIN_SPEED, INPUT_PULLUP);
  pinMode(PIN_SPEEDGND, OUTPUT);
  digitalWrite(PIN_SPEEDGND, LOW);

  cadenceMeter.reset();
  lastLoop = 0;
}

// Interrupt for cadence sensor
void IRAM_ATTR SmartGymBike::cadenceInt() {
  revCount += 1;
  cadenceMeter.signal();
}

void SmartGymBike::setInterrupt(void (*interrupt)(void)) {
  attachInterrupt(PIN_SPEED, interrupt, FALLING);
}

uint32_t SmartGymBike::getRevCount() {
  return revCount;  
}
void SmartGymBike::resetRevCount() {
  revCount = 0;
}
  
int SmartGymBike::getLevel() {
  return analogRead(PIN_POS);
}

int SmartGymBike::getCadence() {
  return cadenceMeter.getRPM();
}

void SmartGymBike::levelStop() {
  digitalWrite(PIN_MP, LOW);
  digitalWrite(PIN_MN, LOW);
  targetLevel = analogRead(PIN_POS);
  setLevel(targetLevel);
  targetReached = true;
}

void SmartGymBike::levelUp(int step) {

  setLevel(targetLevel + step);
}

void SmartGymBike::levelDown(int step) {

  setLevel(targetLevel - step);
}

void SmartGymBike::setLevel(int level) {

  targetLevel = level;
  if (targetLevel < LEVEL_MIN) targetLevel = LEVEL_MIN;
  if (targetLevel > LEVEL_MAX) targetLevel = LEVEL_MAX;
  targetReached = false;
  Serial.print("Set level to: ");
  Serial.println(targetLevel);
}

void SmartGymBike::doLoop() {

  if (millis() - lastLoop < 5) return;  // Do not rush
  lastLoop = millis();
  int mpos = analogRead(PIN_POS);
  
  // Set level to target
  if (!targetReached) {
    if (targetLevel - mpos > 30) {
      digitalWrite(PIN_MP, HIGH);
      digitalWrite(PIN_MN, LOW);
      Serial.print(targetLevel);
      Serial.print(":");
      Serial.print(mpos);
      Serial.println("+");      
    } 
    else if (mpos - targetLevel > 30) {
      digitalWrite(PIN_MN, HIGH);
      digitalWrite(PIN_MP, LOW);
      Serial.print(targetLevel);
      Serial.print(":");
      Serial.print(mpos);
      Serial.println("-");
    }
    else {
      digitalWrite(PIN_MP, LOW);
      digitalWrite(PIN_MN, LOW);
      Serial.print(targetLevel);
      Serial.print(":");
      Serial.print(mpos);
      Serial.println("=");
      targetReached = true;
    }
  }
  
  //if (targetReached) {
  //  if (targetLevel - mpos > 100) targetReached = false;
  //  if (mpos - targetLevel > 100) targetReached = false;
  //}
}

bool SmartGymBike::buttonPressed() {
  return (testButton(BUTTON_UP) || testButton(BUTTON_DOWN));
}

bool SmartGymBike::testButton(int pin) {
  return digitalRead(pin) == LOW;
}
