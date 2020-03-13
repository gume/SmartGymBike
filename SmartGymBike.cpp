#include "SmartGymBike.h"
#include "Button.h"

SmartGymBike::SmartGymBike() :
  buttonUp(PIN_BTNU), buttonDown(PIN_BTND), buttonSS(PIN_BTNS),
  buttonUser(PIN_BTNQ), buttonEnter(PIN_BTNE), buttonPulse(PIN_BTNP) {

  // Setup level controller
  pinMode(PIN_MP, OUTPUT);
  pinMode(PIN_MN, OUTPUT);
  digitalWrite(PIN_MP, LOW);
  digitalWrite(PIN_MN, LOW);

  // Setup level sensor
  //pinMode(PIN_POS, INPUT);

  targetLevel = analogRead(PIN_POS);
  targetReached = true;

  // Setup cadence sensor
  pinMode(PIN_SPEED, INPUT_PULLUP);
  //pinMode(PIN_SPEEDGND, OUTPUT);
  //digitalWrite(PIN_SPEEDGND, LOW);

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

  // Check buttons
  buttonUp.check();
  buttonDown.check();
  buttonSS.check();
  buttonUser.check();
  buttonPulse.check();
  buttonEnter.check();
}
