/*
  BasicTelemetry - publish a sensor reading to Node-RED every 2 seconds.

  This is the minimal example: a PLAIN, LOCAL broker with NO TLS and NO login.
  >> Using HiveMQ Cloud (or any cloud broker)? It needs TLS + username/password.
  >> Use the "HiveMQCloud" example instead, or add .secure().login(...) below.

  Node-RED: add an MQTT-in node subscribed to  devices/esp32-demo/temperature
  Its msg.payload will be JSON: {"value":24.5,"device":"esp32-demo","ts":12345}

  Library: N-R_ESP32   (needs PubSubClient installed)
*/

#include <NodeBridge.h>

NodeBridge bridge;

void setup() {
  Serial.begin(115200);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")     // IP of your MQTT broker / Node-RED host
        .debug(true);               // print status to Serial (optional)

  bridge.begin("esp32-demo");       // this device's name
}

void loop() {
  bridge.loop();                    // keeps WiFi + MQTT alive, auto-reconnects

  static unsigned long last = 0;
  if (millis() - last > 2000) {
    last = millis();

    float tempC = 20.0 + (millis() % 1000) / 100.0;   // fake reading
    bridge.send("temperature", tempC);
  }
}
