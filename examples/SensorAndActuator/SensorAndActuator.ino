/*
  SensorAndActuator - a complete little node:
    * publishes temperature + a button state to Node-RED
    * receives an "led" command and a "threshold" setpoint from Node-RED
    * sends a heartbeat (rssi / uptime) every 30 s
    * reports online/offline presence automatically

  This is the "focus on your algorithm" example - notice how little plumbing
  there is. Everything below setup() is your logic.

  Library: N-R_ESP32   (needs PubSubClient installed)
*/

#include <NodeBridge.h>

#define LED_PIN    2
#define BUTTON_PIN 0     // BOOT button on many ESP32 boards

NodeBridge bridge;

float threshold = 25.0;  // updatable from Node-RED

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        // .login("mqttuser", "mqttpass")   // uncomment if your broker needs auth
        .heartbeat(30000)                    // publish rssi/uptime every 30 s
        .debug(true);

  // ---- commands coming FROM Node-RED ----
  bridge.on("led", [](Value v) {
    digitalWrite(LED_PIN, v.asBool() ? HIGH : LOW);
  });

  bridge.on("threshold", [](Value v) {
    threshold = v.asFloat();
    Serial.printf("New threshold: %.1f\n", threshold);
  });

  bridge.onConnect([]() { Serial.println("** linked to Node-RED **"); });

  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();

  // ---- your algorithm: publish data TO Node-RED ----
  static unsigned long last = 0;
  if (millis() - last > 2000) {
    last = millis();

    float tempC = 20.0 + (millis() % 1000) / 100.0;      // fake sensor
    bool  pressed = (digitalRead(BUTTON_PIN) == LOW);

    bridge.send("temperature", tempC);
    bridge.send("button", pressed);
    bridge.send("alarm", tempC > threshold);             // derived value
  }
}
