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
#define SCREENS 5
typedef void (*screenCallBack)();
screenCallBack screenDisplay[] = { &bikeDisplay_statScreen0,
  &bikeDisplay_statScreen1, &bikeDisplay_statScreen2,
  &bikeDisplay_levelSetScreen, &bikeDisplay_aboutScreen};
screenCallBack screenInit[] = { NULL, NULL, NULL, &bikeDisplay_levelSetScreenInit, NULL };
screenCallBack screenLeave[] = { NULL, NULL, NULL, &bikeDisplay_levelSetScreenLeave, NULL };


#define PRESETLEVELS 6
int presetLevels[] = { 100, 1000, 1750, 2000, 2500, 3000 };
int presetLevel = 0;
  
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

  bike.buttonUser.onClick(&bikeDisplay_screenChange);
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

  screenDisplay[screenMode]();
  
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

void bikeDisplay_showBikeTime(int ypos) {

  // Display bikeTime
  int h = bikeTime / 1000 / 60 / 60;
  int m = (bikeTime / 1000 / 60) % 60;
  int s = (bikeTime / 1000) % 60;
  if (h > 0) {
    display.setCursor(0, ypos);
    display.write(String(h).c_str());
    display.setCursor(4,ypos);
    display.write(":");
  }
  String ms = "0";
  if (m > 9) ms = String(m);
  else ms = ms + String(m);
  display.setCursor(8,ypos);
  display.write(ms.c_str());
  display.setCursor(17,ypos);
  display.write(":");
  String ss = "0";
  if (s > 9) ss = String(s);
  else ss = ss + String(s);
  display.setCursor(21,ypos);
  display.write(ss.c_str());
  display.drawLine(0, 9+ypos, 31, 9+ypos, SSD1306_WHITE);
}

void bikeDisplay_statScreen0() {

  display.clearDisplay();
  bikeDisplay_showBikeTime(0);
  
  // Display Level
  bikeDisplay_writeCenter("Level", 21);
  bikeDisplay_writeCenter(String(bikeLevel), 30);

  // Display Cadence
  bikeDisplay_writeCenter("RPM", 51);
  bikeDisplay_writeCenter(String(bikeCadence), 60);

  // Display Revs
  bikeDisplay_writeCenter("Revs", 81);
  bikeDisplay_writeCenter(String(bikeRevs), 90);
}

void bikeDisplay_statScreen1() {

  display.clearDisplay();
  bikeDisplay_showBikeTime(0);

  // Three bars
  bikeDisplay_drawProgressbarH(1, 12, 14, 85, bikeLevel/32);
  bikeDisplay_drawProgressbarH(17, 12, 14, 85, bikeCadence-20);
  bikeDisplay_writeCenter(String(bikeRevs), 105);
}

void bikeDisplay_statScreen2() {

  int m = (bikeTime / 1000 / 60) % 60;
  int s = (bikeTime / 1000) % 60;

  String ms = "0";
  if (m > 9) ms = String(m);
  else ms = ms + String(m);
  String ss = "0";
  if (s > 9) ss = String(s);
  else ss = ss + String(s);
    
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 40);
  display.write(ms.c_str());  
  display.setCursor(0, 70);
  display.write(ss.c_str());
  display.setTextSize(1);
}

void bikeDisplay_levelSetScreen() {

  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0,30);
  display.write(String("L"+String(presetLevel)).c_str());
  display.setTextSize(1);
  bikeDisplay_writeCenter(String(presetLevels[presetLevel]), 70);
}

void bikeDisplay_aboutScreen() {

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.write("Version");
  display.setCursor(0,10);
  display.write(bikeRikeVersion);
  display.setCursor(0,20);
  display.write("IP");
  display.setCursor(0,30);
  display.write(WiFi.localIP());
}

void bikeDisplay_levelSetScreenInit() {
  bike.buttonUp.hideState(true);
  bike.buttonUp.onClick(bikeDisplay_levelChange);
  bike.buttonDown.hideState(true);
  bike.buttonDown.onClick(bikeDisplay_levelChange);
  bike.buttonSS.hideState(true);
  bike.buttonSS.onClick(bikeDisplay_levelChange);
}

void bikeDisplay_levelSetScreenLeave() {
  bike.buttonUp.hideState(false);
  bike.buttonUp.onClick(NULL);
  bike.buttonDown.hideState(false);
  bike.buttonDown.onClick(NULL);  
  bike.buttonSS.hideState(false);
  bike.buttonSS.onClick(NULL);
}

void bikeDisplay_levelChange(int bp) {
  if (bp == PIN_BTNU) {
    if (++presetLevel >= PRESETLEVELS) presetLevel = 0;
  }
  else if (bp == PIN_BTND) {
    if (--presetLevel < 0) presetLevel = PRESETLEVELS - 1;
  }
  bike.setLevel(presetLevels[presetLevel]);
}

void bikeDisplay_screenChange(int bp) {
  if (screenLeave[screenMode]) screenLeave[screenMode]();
  if (++screenMode >= SCREENS) screenMode = 0;
  if (screenInit[screenMode]) screenInit[screenMode]();
}

void bikeDisplay_drawProgressbarH(int x,int y, int width, int height, int progress) {
   progress = progress > 100 ? 100 : progress; // set the progress value to 100
   progress = progress < 0 ? 0 :progress; // start the counting to 0-100
   int bar = ((height-4) * progress) / 100;
   display.drawRect(x, y, width, height, SSD1306_WHITE);
   display.fillRect(x+2, y+height-2-bar, width-4, bar, SSD1306_WHITE);
}

void bikeDisplay_writeCenter(String text, int ypos) {
  display.setCursor(16-(text.length()*5/2),ypos);
  display.write(text.c_str());
}
