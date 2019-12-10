#include <IotWebConf.h>

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to build an AP. (E.g. in case of lost password)
#define CONFIG_PIN 22
// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN 2

HTTPUpdateServer httpUpdater;

IotWebConfParameter mqttServerParam = IotWebConfParameter("MQTT server", "mqttServer", mqttServerValue, STRING_LEN);
IotWebConfParameter mqttUserNameParam = IotWebConfParameter("MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN);
IotWebConfParameter mqttUserPasswordParam = IotWebConfParameter("MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN, "password");

void webIF_setup() {

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addParameter(&mqttServerParam);
  iotWebConf.addParameter(&mqttUserNameParam);
  iotWebConf.addParameter(&mqttUserPasswordParam);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  iotWebConf.setupUpdateServer(&httpUpdater);

  // -- Initializing the configuration.
  boolean validConfig = iotWebConf.init();
  if (!validConfig)
  {
    mqttServerValue[0] = '\0';
    mqttUserNameValue[0] = '\0';
    mqttUserPasswordValue[0] = '\0';
  }
  
  // -- Set up required URL handlers on the web server.
  server.on("/", webIF_handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.on("/api/get", HTTP_GET, webIF_handleGetApi);
  server.on("/api/set", HTTP_POST, webIF_handleSetApi);
  
  server.onNotFound([](){ iotWebConf.handleNotFound(); });
}

void webIF_doLoop() {
  iotWebConf.doLoop();
}


/**
 * Handle web requests to "/" path.
 */
void webIF_handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>SmartGymBike MQTT App</title></head><body>MQTT SmartGymBike";
  s += "<ul>";
  s += "<li>MQTT server: ";
  s += mqttServerValue;
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void webIF_handleGetApi() {
  if (server.hasArg("level")) {
    server.send(200, "application/json", "{ \"level\": " + String(bike.getLevel()) + " }");
    return;
  }
  if (server.hasArg("cadence")) {
    server.send(200, "application/json", "{ \"cadence\": " + String(bike.getCadence()) + " }");
    return;
  }
  if (server.hasArg("revs")) {
    server.send(200, "application/json", "{ \"revs\": " + String(bike.getRevCount()) + " }");
    return;
  }

  server.send(200, "application/json",
    "{ \"level\": " + String(bike.getLevel()) + ", " + 
    "\"cadence\": " + String(bike.getCadence()) + ", " + 
    "\"revs\": " + String(bike.getRevCount()) + " }");
}

void webIF_handleSetApi() {
  String postStr = server.arg("plain");
  StaticJsonDocument<200> a;
  DeserializationError error = deserializeJson(a, postStr);
  if (!error) {
    if (a.containsKey("level")) {
      bike.setLevel(a["level"]);
      server.send(200, "application/json", "{}");
      return;
    }
    else if (a.containsKey("reset")) {
      bike.resetRevCount();
      server.send(200, "application/json", "{}");
      return;
    }
  }

  server.send(404, "404 Not Found");
}
