#include "FreqCount.h"
#include "Arduino.h"

FreqCounter::FreqCounter() {

  tbuffer.clear();
}

void FreqCounter::signal() {
  tbuffer.push(millis());
}

int FreqCounter::getRPM() {
  if (tbuffer.isEmpty()) return 0;

  uint32_t now =  millis();
  uint32_t df = 0; uint32_t dc = 0; uint32_t dl = 0;
  for (int i = 0; i < tbuffer.size(); i++) {
    uint32_t diff = now - tbuffer[i];
    if (diff > TOOOLD) continue;
    if (df == 0) df = tbuffer[i];
    dl = tbuffer[i];
    dc = dc + 1;
  }
  if (dc < 2) return 0;

  float avgMsLapse = (float) (dl - df) / (float) (dc - 1);
  float hz = 1000.0 / avgMsLapse;
  float rpm = hz * 60.0; // 1 HZ = 60 RPM

  return (int) rpm;
}

void FreqCounter::reset() {
  tbuffer.clear();
}
