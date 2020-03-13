#pragma once

#include "FreqCount.h"
#include "Button.h"
#include "Arduino.h"

#define PIN_BTNQ 15
#define PIN_BTNP 16
#define PIN_BTNS 17
#define PIN_BTNE 23
#define PIN_BTND 19
#define PIN_BTNU 18

#define PIN_MP 26
#define PIN_MN 27

#define PIN_POS 33
#define PIN_SPEED 32

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

  Button buttonUp;
  Button buttonDown;
  Button buttonSS;
  Button buttonUser;
  Button buttonEnter;
  Button buttonPulse;
    
  void doLoop();

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
