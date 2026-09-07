/*
  GroveMoisture - Grove Moisture Sensor (analog, soil resistivity)
  https://wiki.seeedstudio.com/Grove-Moisture_Sensor/

  Works on a Seeed XIAO ESP32 (+ XIAO Expansion Board) and on the Arduino
  UNO R4 WiFi (+ Grove Base Shield). Wiring: plug into an *A0* (analog) Grove port.

  Publishes every second:
    devices/esp32-demo/moisture      0-100 %   (0 = bone dry, 100 = soaked)
    devices/esp32-demo/moistureRaw   raw ADC reading (for calibration)

  The sensor is resistive: WETTER soil conducts more, so the raw reading goes UP
  with moisture. Absolute numbers depend on the board's ADC and your soil, so you
  MUST calibrate the two constants below once. The Seeed wiki quotes (on a 10-bit,
  5 V Arduino): 0-300 dry, 300-700 humid, 700-950 in water.

  Libraries: N-R_ESP32, PubSubClient.
*/

#include <NodeBridge.h>

#define MOISTURE_PIN A0

// ---- calibrate these once for your board + soil -----------------------------
// 1) Read moistureRaw with the probe in DRY soil (or air)  -> put that in RAW_DRY
// 2) Read moistureRaw with the probe in WET soil (or water) -> put that in RAW_WET
// Defaults are rough values for an ESP32 12-bit ADC (0..4095). On a 10-bit board
// (UNO R4 default) they are closer to ~0 dry and ~700 wet — measure and adjust.
int RAW_DRY = 0;
int RAW_WET = 3000;

NodeBridge bridge;

void setup() {
  Serial.begin(115200);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")     // your MQTT broker / Node-RED host
        .debug(true);
  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();

  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();

    int raw = analogRead(MOISTURE_PIN);
    int pct = map(raw, RAW_DRY, RAW_WET, 0, 100);
    pct = constrain(pct, 0, 100);

    const char* label = pct < 30 ? "dry" : (pct < 70 ? "humid" : "wet");
    if (bridge.connected()) { Serial.print("moisture "); Serial.print(pct);
                              Serial.print("% ("); Serial.print(label); Serial.println(")"); }

    bridge.send("moisture", pct);
    bridge.send("moistureRaw", raw);
  }
}
