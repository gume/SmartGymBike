#pragma once

#include <CircularBuffer.h>

class FreqCounter {
public:
  FreqCounter();

  void reset();
  void signal();
  int getRPM();

  #define BUFFSIZE 10
  #define TOOOLD 3000
private:
  CircularBuffer<uint32_t, BUFFSIZE> tbuffer;
};