/*
  ExpansionBuzzer - the passive buzzer built into the XIAO Expansion Board (pin A3),
  controlled FROM Node-RED.

  Node-RED: publish to  devices/esp32-demo/buzzer/set
            "on"  -> beep,  "off" -> silence,  or a number -> that frequency (Hz).

  Libraries: N-R_ESP32, PubSubClient.
*/

#include <NodeBridge.h>

#define BUZZER_PIN D3          // onboard passive buzzer on the XIAO Expansion Board (pin 3)

NodeBridge bridge;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);

  bridge.on("buzzer", [](Value v) {
    const char* s = v.asString();
    if (!strcasecmp(s, "off")) { noTone(BUZZER_PIN); return; }
    int freq = v.asInt();                       // "on" -> 0 -> default tone
    tone(BUZZER_PIN, freq > 0 ? freq : 1000, 200);
  });

  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();
}
