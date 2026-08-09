/*
  GroveTempHumidity - Grove Temperature & Humidity Sensor V2.0 (DHT20 / AHT20)
  on a Seeed Studio XIAO ESP32 + XIAO Expansion Board.

  Wiring: plug the Grove module into the Expansion Board's *I2C* Grove port.
  Publishes to Node-RED:  devices/esp32-demo/temperature  and  .../humidity

  Libraries: N-R_ESP32, PubSubClient, DHT20 (by Rob Tillaart).
*/

#include <Wire.h>
#include "DHT20.h"
#include <NodeBridge.h>

NodeBridge bridge;
DHT20 dht;

void setup() {
  Serial.begin(115200);
  Wire.begin();             // XIAO default I2C (D4=SDA, D5=SCL)
  dht.begin();

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")     // your broker / Node-RED host
        .debug(true);
  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();

  static unsigned long last = 0;
  if (millis() - last > 2000) {
    last = millis();
    if (dht.read() == DHT20_OK) {
      bridge.send("temperature", dht.getTemperature());
      bridge.send("humidity",    dht.getHumidity());
    } else {
      Serial.println("DHT20 read failed - check the I2C Grove connection");
    }
  }
}
