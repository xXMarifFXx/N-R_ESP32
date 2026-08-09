/*
  GrovePIRMotion - Grove Mini PIR Motion Sensor (digital)
  on a Seeed Studio XIAO ESP32 + XIAO Expansion Board.

  Wiring: plug into the Expansion Board's *A0/D0* Grove port (used as digital).
  Publishes:  devices/esp32-demo/motion   (true on movement, edge-triggered)

  Libraries: N-R_ESP32, PubSubClient.
*/

#include <NodeBridge.h>

#define PIR_PIN D0

NodeBridge bridge;
bool lastMotion = false;

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);
  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();

  bool motion = digitalRead(PIR_PIN) == HIGH;
  if (motion != lastMotion) {          // publish only on change
    lastMotion = motion;
    bridge.send("motion", motion);
  }
}
