#include <WS2812FX.h>
#include "ESP32_RMT_Driver.h"

#define BIKELED_NONE    0
#define BIKELED_BREATH  1
#define BIKELED_RUNNING 2
int bikeLED_mode = 0;

WS2812FX ws2812fx = WS2812FX(3, 25, NEO_GRB  + NEO_KHZ800); // 3 RGB LEDs driven by GPIO_25

void bikeLED_rmtShow(void) {
  uint8_t *pixels = ws2812fx.getPixels();
  uint16_t numBytes = ws2812fx.getNumBytes() + 1;
  rmt_write_sample(RMT_CHANNEL_0, pixels, numBytes, false); // channel 0
}

void bikeLED_setup() {
  ws2812fx.init();
  ws2812fx.setBrightness(64);
  rmt_tx_int(RMT_CHANNEL_0, ws2812fx.getPin());
  ws2812fx.setCustomShow(bikeLED_rmtShow);
}

void bikeLED_doLoop() {
  ws2812fx.service();
}

void bikeLED_breath(int color) {
  ws2812fx.setSegment(0, 0, 2, FX_MODE_BREATH, color,  1000, NO_OPTIONS);
  ws2812fx.start();
  bikeLED_mode = BIKELED_BREATH;
}

void bikeLED_running(int cadence, int level) {
  if (bikeLED_mode != BIKELED_RUNNING) {
    ws2812fx.setSegment(0, 0, 2, FX_MODE_SCAN, BLACK,  10000, NO_OPTIONS);
    ws2812fx.start();
    bikeLED_mode = BIKELED_RUNNING;
  }

  byte r, g, b;
  int c = ((uint32_t) level * (uint32_t) 254) / SmartGymBike::LEVEL_MAX;
  b = max(127-c,0)*2;
  g = 255-abs(127-c)*2;
  r = max(c-127,0)*2;
  ws2812fx.setColor(((uint32_t)r << 16) + ((uint32_t)g << 8) + (uint32_t)b);
  //ws2812fx.setColor(GREEN);

  int ls = 5000 - cadence*40;
  if (ls < 10) ls = 10;
  ws2812fx.setSpeed(ls);
}
