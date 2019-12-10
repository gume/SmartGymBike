#pragma once

#include "FreqCount.h"
#include "Arduino.h"

#define BTN_G1 23
#define BTN_G2 5
#define BTN_Q 22
#define BTN_P 1
#define BTN_S 3
#define BTN_E 21
#define BTN_D 19
#define BTN_U 18

#define PIN_MP 27
#define PIN_MN 26

#define PIN_POSVCC 25
#define PIN_POS 33
#define PIN_POSGND 32

#define PIN_SPEED 16
#define PIN_SPEEDGND 17

class SmartGymBike {
public:
  SmartGymBike();

  uint32_t getRevCount();
  void resetRevCount();
  int getLevel();
  int getCadence();

  void levelUp(int step = 100);
  void levelDown(int step = 100);
  void setLevel(int level);
  void levelStop();

  bool buttonPressed();
  bool testButton(int button);
  
  void doLoop();
  
  const static int BUTTON_UP = BTN_U;
  const static int BUTTON_DOWN = BTN_D;

  const static int LEVEL_MAX = 3200;  // Very instable at 4000
  const static int LEVEL_MIN = 100;

  void IRAM_ATTR cadenceInt();
  void setInterrupt(void (*interrupt)(void));
  
private:
  uint32_t revCount;
  int targetLevel;  
  bool targetReached;
  FreqCounter cadenceMeter;
  uint32_t lastLoop;
};
