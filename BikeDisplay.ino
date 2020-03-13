#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool event;
uint32_t eventTime;  // When a dipsplay event happened
uint32_t eventHoldTime; // How long display will stay

uint32_t lastRefresh; // Last time display was refreshed
uint32_t refreshInterval; // refresh period time

int screenMode;  // Display various screens

void bikeDisplay_setup() {
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("ERROR! SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.setRotation(1);

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setTextWrap(false);
  
  refreshInterval = 500;  // 2 FPS
  event = false;
  screenMode = 0;
}

void bikeDisplay_doLoop() {

  uint32_t now = millis();
  // Check display event
  if (event) {
    if (now - eventTime < eventHoldTime) {
      // Do nothing 
      return;
    } else {
      event = false;
    }
  }

  // Check refresh
  if (now - lastRefresh < refreshInterval) {
    // Do nothing
    return;
  }

  switch (screenMode) {
  case 0:
    bikeDisplay_statScreen0();
    break;
  }
  
  // Display FPS
  int fps10 = 10000 / (now - lastRefresh);
  display.setCursor(0,120);
  if (fps10 < 100) {
    display.write(String(fps10/10).c_str());
    display.drawPixel(5,126,SSD1306_WHITE);
    display.drawPixel(5,127,SSD1306_WHITE);
    display.setCursor(7,120);
    display.write(String(fps10 % 10).c_str());
  } else {
    display.write(String(fps10/10).c_str());
  }
  display.setCursor(14,120);
  display.write("FPS");

  display.display();
  
  lastRefresh = now;
}

void bikeDisplay_toast(String message, uint32_t duration) {
  // Display toast immediately
  eventTime = millis();
  eventHoldTime = duration;
  event = true;
  
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner
  display.write(message.c_str());
  display.display();
}

void bikeDisplay_statScreen0() {

  display.clearDisplay();
  // Display bikeTime
  int h = bikeTime / 1000 / 60 / 60;
  int m = (bikeTime / 1000 / 60) % 60;
  int s = (bikeTime / 1000) % 60;
  if (h > 0) {
    display.setCursor(0, 0);
    display.write(String(h).c_str());
    display.setCursor(4,0);
    display.write(":");
  }
  String ms = "0";
  if (m > 9) ms = String(m);
  else ms = ms + String(m);
  display.setCursor(8,0);
  display.write(ms.c_str());
  display.setCursor(17,0);
  display.write(":");
  String ss = "0";
  if (s > 9) ss = String(s);
  else ss = ss + String(s);
  display.setCursor(21,0);
  display.write(ss.c_str());
  display.drawLine(0, 9, 31, 9, SSD1306_WHITE);

  // Display Level
  display.setCursor(3,11);
  display.write("Level");
  display.setCursor(3,19);
  display.write(String(bikeLevel).c_str());

  // Display Cadence
  display.setCursor(3,31);
  display.write("RPM");
  display.setCursor(3,39);
  display.write(String(bikeCadence).c_str());

  // Display Revs
  display.setCursor(3,51);
  display.write("Revs");
  display.setCursor(3,59);
  display.write(String(bikeRevs).c_str());
}
