/*
  GroveRotaryAngle - Grove Rotary Angle Sensor (analog potentiometer)
  on a Seeed Studio XIAO ESP32 + XIAO Expansion Board.

  Wiring: plug into the Expansion Board's *A0/D0* (analog) Grove port.
  Publishes:  devices/esp32-demo/rotary   (0-100 %)

  Libraries: N-R_ESP32, PubSubClient.
*/

#include <NodeBridge.h>

#define ROTARY_PIN A0

NodeBridge bridge;
int lastPct = -1;

void setup() {
  Serial.begin(115200);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);
  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();

  static unsigned long last = 0;
  if (millis() - last > 200) {
    last = millis();
    int pct = map(analogRead(ROTARY_PIN), 0, 4095, 0, 100);
    if (pct != lastPct) {            // publish only on change - less traffic
      lastPct = pct;
      bridge.send("rotary", pct);
    }
  }
}
