/*
  MariffbPortal — known-good example for https://mqtt.mariffb.my

  Register in the portal first. Replace WiFi, username and password below.
  IMPORTANT: MQTT_USERNAME is also passed to begin(); the portal authorizes only
  devices/<username>/#. The library still creates a unique MQTT client ID from
  this board's chip ID, so Node-RED and the portal console do not collide with it.
*/

#include <WiFi.h>
#include <time.h>
#include <NodeBridge.h>
#include <NodeBridgeCerts.h>

const char* WIFI_NAME = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_USERNAME = "YOUR_PORTAL_USERNAME";
const char* MQTT_PASSWORD = "YOUR_PORTAL_PASSWORD";

NodeBridge bridge;

void setup() {
  Serial.begin(115200);

  // TLS certificate validation needs a roughly correct clock.
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  unsigned long wifiStarted = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStarted < 15000) delay(200);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  unsigned long timeStarted = millis();
  while (time(nullptr) < 1700000000 && millis() - timeStarted < 15000) delay(200);

  bridge.wifi(WIFI_NAME, WIFI_PASSWORD)
        .broker("mqtt.mariffb.my", 8883)
        .secure(NODEBRIDGE_ISRG_ROOT_X1)
        .login(MQTT_USERNAME, MQTT_PASSWORD)
        .keepAlive(60)
        .debug(true);

  bridge.begin(MQTT_USERNAME);  // topics: devices/<MQTT_USERNAME>/#
}

void loop() {
  bridge.loop();

  static unsigned long last = 0;
  if (millis() - last >= 2000) {
    last = millis();
    bridge.send("temperature", 24.5);
  }
}
