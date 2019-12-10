#include <MQTT.h>
#include <ArduinoJson.h>
#include "SmartGymBike.h"
#include <IotWebConf.h>
#include <Restfully.h>

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
RestRequestHandler restHandler;

#define STRING_LEN 128
char mqttServerValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];

boolean needMqttConnect = false;
unsigned long lastReport = 0;
unsigned long lastMqttConnectionAttempt = 0;
boolean pressed = false;

void IRAM_ATTR bikeInterrupt() {
  bike.cadenceInt();
}

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

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
  
  if (needMqttConnect) {
    if (connectMqtt()) {
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
    lastReport = now;
    
    int valL = bike.getLevel();
    Serial.print("Level: ");
    Serial.println(valL);
    mqttClient.publish("/smartgymbike/level", String(valL));

    int valC = bike.getCadence();
    Serial.print("Cadence: ");
    Serial.println(valC);
    mqttClient.publish("/smartgymbike/cadence", String(valC));

    int valR = bike.getRevCount();
    Serial.print("Rev count: ");
    Serial.println(valR);
    mqttClient.publish("/smartgymbike/revs", String(valR));
  }

  if (bike.buttonPressed()) {

    pressed = true;
    if (bike.testButton(SmartGymBike::BUTTON_UP))
      bike.setLevel(SmartGymBike::LEVEL_MAX);

    if (bike.testButton(SmartGymBike::BUTTON_DOWN))
      bike.setLevel(SmartGymBike::LEVEL_MIN);
  }

  if (pressed && !bike.buttonPressed()) {
    pressed = false;
    bike.levelStop();
  }
}

void wifiConnected()
{
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
