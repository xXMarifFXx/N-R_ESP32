/*
  UnoR4Telemetry — N-R_ESP32 on the Arduino UNO R4 WiFi

  The SAME NodeBridge API works on the UNO R4 WiFi and on the ESP32 family, so a
  sketch written for one board runs on the other. The only TLS difference is
  handled for you:

    * ESP32   -> pass a root CA with .secure(NODEBRIDGE_ISRG_ROOT_X1) to verify.
    * UNO R4  -> just call .secure(): the radio module verifies the server against
                 its own built-in CA bundle. No root CA and no NTP clock needed.

  This example targets the class broker (mqtt.mariffb.my) over TLS. Register in
  the portal first, then paste your WiFi + portal username/password below.

  Node-RED: subscribe an MQTT-in node to  devices/<your-username>/temperature
  Library: N-R_ESP32   (needs PubSubClient installed)
*/

#include <NodeBridge.h>

const char* WIFI_NAME     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_USERNAME = "YOUR_PORTAL_USERNAME";   // also the device name (ACL: devices/<username>/#)
const char* MQTT_PASSWORD = "YOUR_PORTAL_PASSWORD";

NodeBridge bridge;

void setup() {
  Serial.begin(115200);

  bridge.wifi(WIFI_NAME, WIFI_PASSWORD)
        .broker("mqtt.mariffb.my", 8883)   // TLS; use broker("192.168.1.10") for a plain LAN broker
        .secure()                          // UNO R4 verifies via its built-in CA bundle
        .login(MQTT_USERNAME, MQTT_PASSWORD)
        .keepAlive(60)
        .debug(true);

  bridge.begin(MQTT_USERNAME);             // topics: devices/<MQTT_USERNAME>/#
}

void loop() {
  bridge.loop();

  static unsigned long last = 0;
  if (millis() - last >= 2000) {
    last = millis();
    float tempC = 20.0 + (millis() % 1000) / 100.0;   // fake reading — replace with your sensor
    bridge.send("temperature", tempC);
  }
}
