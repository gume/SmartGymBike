#include <MQTT.h>
#include <ArduinoJson.h>
#include "SmartGymBike.h"
#include <IotWebConf.h>

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

#define STRING_LEN 128
char mqttServerValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];

boolean needMqttConnect = false;
uint32_t lastReport = 0;
uint32_t lastMqttConnectionAttempt = 0;
boolean pressed = false;
boolean setupMode;
uint32_t lastActiveTime = 0;

int bikeLevel = 0;
int bikeCadence = 0;
int bikeRevs = 0;
int pBikeRevs = 0;
uint32_t bikeTime = 0;

bool recoveryMode = false;

void IRAM_ATTR bikeInterrupt() {
  bike.cadenceInt();
}


void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  setupMode = true;
  webIF_setup();

  if (bike.buttonPulse.pressed()) {
    Serial.println("Recovery mode");
    recoveryMode = true;
    return;
  }

  bikeLED_setup();
  bikeLED_breath(0x0000ff); // Blue

  bikeDisplay_setup();
  bikeDisplay_toast("Welc\nome!", 3000);  
  
  mqttClient.begin(mqttServerValue, net);
  mqttClient.onMessage(mqttMessageReceived);

  bike.setInterrupt(bikeInterrupt);

  BleIF_setup();

  Serial.println("Ready.");
}

void loop() 
{
  if (recoveryMode) {
    iotWebConf.doLoop();
    return;
  }

  // -- doLoop should be called as frequently as possible.
  webIF_doLoop();
  mqttClient.loop();
  bike.doLoop();
  bikeLED_doLoop();
  bikeDisplay_doLoop();
  
  if (needMqttConnect) {
    bikeLED_breath(0xff0000); // Red
    if (connectMqtt()) {
      bikeDisplay_toast("MQTT\nOK!", 2000);
      needMqttConnect = false;
      setupMode = false;
    }
  }
  else if ((iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE) && (!mqttClient.connected()))
  {
    Serial.println("MQTT reconnect");
    setupMode = true;
    needMqttConnect = true;
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

    if (bikeRevs > pBikeRevs) {
      pBikeRevs = bikeRevs;
      BleIF_update(bikeRevs);
    }

    if (bikeCadence > 0) {
      bikeTime = bikeTime + (now - lastReport);
      lastActiveTime = now;
    }
    mqttClient.publish("/smartgymbike/time", String(bikeTime/1000));
    
    lastReport = now;
  }

  // Set LEDs
  if (!setupMode) bikeLED_running(bikeCadence, bikeLevel);

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

  // Reset button (long SS)
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
  mqttClient.publish("/smartgymbike/info", "started");

  mqttClient.subscribe("/smartgymbike/control");
  return true;
}

boolean connectMqttOptions()
{
  boolean result;
  if (mqttUserPasswordValue[0] != '\0') {
    result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue, mqttUserPasswordValue);
  }
  else if (mqttUserNameValue[0] != '\0') {
    result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue);
  }
  else {
    result = mqttClient.connect(iotWebConf.getThingName());
  }
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
    mqttClient.publish("/smartgymbike/info", "level set to " + String(target));
  }
}
