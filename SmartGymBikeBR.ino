#include <MQTT.h>
#include <ArduinoJson.h>
#include "SmartGymBike.h"
#include <IotWebConf.h>

#include <WS2812FX.h>
#include "ESP32_RMT_Driver.h"


// -- Initial name of the Thing. SSID of the own Access Point.
const char thingName[] = "Szmardzsimbájk";
// -- Initial password to connect to the Thing
const char wifiInitialApPassword[] = "bikerike";

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "mqt1"

// -- Callback method declarations.
void wifiConnected();
void mqttMessageReceived(String &topic, String &payload);

DNSServer dnsServer;
WiFiClient net;
MQTTClient mqttClient;
SmartGymBike bike;
WebServer server(80);
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
//RestRequestHandler restHandler;
WS2812FX ws2812fx = WS2812FX(3, 25, NEO_GRB  + NEO_KHZ800); // 3 RGB LEDs driven by GPIO_25

#define STRING_LEN 128
char mqttServerValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];

boolean needMqttConnect = false;
unsigned long lastReport = 0;
unsigned long lastMqttConnectionAttempt = 0;
boolean pressed = false;

int bikeLevel = 0;
int bikeCadence = 0;
int bikeRevs = 0;
uint32_t bikeTime = 0;

void IRAM_ATTR bikeInterrupt() {
  bike.cadenceInt();
}

void rmtShow(void) {
  uint8_t *pixels = ws2812fx.getPixels();
  uint16_t numBytes = ws2812fx.getNumBytes() + 1;
  rmt_write_sample(RMT_CHANNEL_0, pixels, numBytes, false); // channel 0
}


void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  ws2812fx.init();
  ws2812fx.setBrightness(64);
  rmt_tx_int(RMT_CHANNEL_0, ws2812fx.getPin());
  ws2812fx.setCustomShow(rmtShow);
  ws2812fx.setSegment(0, 0, 2, FX_MODE_BREATH, BLUE,  1000, NO_OPTIONS);
  ws2812fx.start();

  bikeDisplay_setup();
  bikeDisplay_toast("Welcome!", 3000);  

  webIF_setup();
  
  mqttClient.begin(mqttServerValue, net);
  mqttClient.onMessage(mqttMessageReceived);

  bike.setInterrupt(bikeInterrupt);

  Serial.println("Ready.");
}

void loop() 
{
  // -- doLoop should be called as frequently as possible.
  webIF_doLoop();
  mqttClient.loop();
  bike.doLoop();
  ws2812fx.service();
  bikeDisplay_doLoop();
  
  if (needMqttConnect) {
    ws2812fx.setSegment(0, 0, 2, FX_MODE_SCAN, RED,  1000, NO_OPTIONS);
    if (connectMqtt()) {
      //ws2812fx.setSegment(0, 0, 2, FX_MODE_STATIC, BLACK,  1, NO_OPTIONS);
      bikeDisplay_toast("MQTT\nOK!", 2000);
      needMqttConnect = false;
    }
  }
  else if ((iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE) && (!mqttClient.connected()))
  {
    Serial.println("MQTT reconnect");
    connectMqtt();
  }

/*  if (needReset)
  {
    Serial.println("Rebooting after 1 second.");
    iotWebConf.delay(1000);
    ESP.restart();
  }*/

  unsigned long now = millis();
  if (500 < now - lastReport) {
    
    bikeLevel = bike.getLevel();
    Serial.print("Level: ");
    Serial.println(bikeLevel);
    mqttClient.publish("/smartgymbike/level", String(bikeLevel));

    bikeCadence = bike.getCadence();
    Serial.print("Cadence: ");
    Serial.println(bikeCadence);
    mqttClient.publish("/smartgymbike/cadence", String(bikeCadence));

    bikeRevs = bike.getRevCount();
    Serial.print("Rev count: ");
    Serial.println(bikeRevs);
    mqttClient.publish("/smartgymbike/revs", String(bikeRevs));

    if (bikeCadence > 0) {
      bikeTime = bikeTime + (now - lastReport);
    }
    int ls = 5000-bikeCadence*40;
    if (ls < 10) ls = 10;
    ws2812fx.setSpeed(ls);
    
    lastReport = now;
  }

  // Level buttons
  if (bike.buttonUp.pressed() || bike.buttonDown.pressed()) {
    pressed = true;
    if (bike.buttonUp.pressed())
      bike.setLevel(SmartGymBike::LEVEL_MAX);

    if (bike.buttonDown.pressed())
      bike.setLevel(SmartGymBike::LEVEL_MIN);
  } else {
    if (pressed) {
      pressed = false;
      bike.levelStop();      
    }
  }

  // Reset button
  if (bike.buttonSS.longPress(5000)) {
     ESP.restart();
  }

}

void wifiConnected()
{
  Serial.println("WiFi connected.");
  bikeDisplay_toast("WiFi\nOK!", 2000);
  needMqttConnect = true;
}

boolean connectMqtt() {
  unsigned long now = millis();
  if (1000 > now - lastMqttConnectionAttempt) {
    // Do not repeat within 1 sec.
    return false;
  }
  Serial.println("Connecting to MQTT server...");
  if (!connectMqttOptions()) {
    lastMqttConnectionAttempt = now;
    return false;
  }
  Serial.println("Connected!");

  mqttClient.subscribe("/smartgymbike/control");
  return true;
}

boolean connectMqttOptions()
{
  boolean result;
  if (mqttUserPasswordValue[0] != '\0')
  {
    result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue, mqttUserPasswordValue);
  }
  else if (mqttUserNameValue[0] != '\0')
  {
    result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue);
  }
  else
  {
    result = mqttClient.connect(iotWebConf.getThingName());
  }

  mqttClient.publish("/smartgymbike/debug", "MQTT started.");
  return result;
}

void mqttMessageReceived(String &topic, String &payload)
{
  Serial.println("Incoming: " + topic + " - " + payload);

  StaticJsonDocument<200> msg;
  DeserializationError error = deserializeJson(msg, payload);
  if (error) return;

  if (msg.containsKey("level")) {
    int target = msg["level"];
    bike.setLevel(target);
    mqttClient.publish("/smartgymbike/debug", "level set to " + String(target));

  }
}
