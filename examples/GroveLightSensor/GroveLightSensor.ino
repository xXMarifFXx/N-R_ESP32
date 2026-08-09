/*
  GroveLightSensor - Grove Light Sensor v1.2 (analog)
  on a Seeed Studio XIAO ESP32 + XIAO Expansion Board.

  Wiring: plug into the Expansion Board's *A0/D0* (analog) Grove port.
  Publishes:  devices/esp32-demo/light   (0-100 %)

  Libraries: N-R_ESP32, PubSubClient.
*/

#include <NodeBridge.h>

#define LIGHT_PIN A0

NodeBridge bridge;

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
  if (millis() - last > 1000) {
    last = millis();
    int raw = analogRead(LIGHT_PIN);          // ESP32 ADC: 0..4095
    int pct = map(raw, 0, 4095, 0, 100);
    bridge.send("light", pct);
  }
}
