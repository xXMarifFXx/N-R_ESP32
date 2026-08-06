/*
  ReceiveCommands - control the ESP32's onboard LED from Node-RED.

  Node-RED: add an MQTT-out node that publishes to
      devices/esp32-demo/led/set
  Send "on" / "off" (or true/false, 1/0), or JSON {"value":"on"}.

  Plain LOCAL broker (no TLS/login). For HiveMQ Cloud or any cloud broker,
  see the "HiveMQCloud" example, or add .secure().login(...) to the config.

  Library: N-R_ESP32   (needs PubSubClient installed)
*/

#include <NodeBridge.h>

#define LED_PIN 2   // onboard LED on most ESP32 dev boards

NodeBridge bridge;

// Handler fires whenever Node-RED sends a command on the "led" key.
void onLed(Value v) {
  digitalWrite(LED_PIN, v.asBool() ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);

  bridge.on("led", onLed);          // subscribe to the "led" command

  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();                    // that's it - commands arrive via onLed()
}
