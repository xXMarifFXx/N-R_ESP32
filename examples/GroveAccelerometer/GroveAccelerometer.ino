/*
  GroveAccelerometer - Grove 3-Axis Digital Accelerometer (LIS3DHTR)
  on a Seeed Studio XIAO ESP32 + XIAO Expansion Board.

  Wiring: plug into the Expansion Board's *I2C* Grove port.
  Publishes:  devices/esp32-demo/accel_x, .../accel_y, .../accel_z  (in g)

  Libraries: N-R_ESP32, PubSubClient,
             Grove-3-Axis-Digital-Accelerometer-2g-to-16g-LIS3DHTR (by Seeed).
*/

#include <Wire.h>
#include "LIS3DHTR.h"
#include <NodeBridge.h>

NodeBridge bridge;
LIS3DHTR<TwoWire> accel;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  accel.begin(Wire, 0x19);                 // Grove LIS3DHTR default address
  accel.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);
  accel.setFullScaleRange(LIS3DHTR_RANGE_2G);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);
  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();

  static unsigned long last = 0;
  if (millis() - last > 500) {
    last = millis();
    if (accel) {
      bridge.send("accel_x", accel.getAccelerationX());
      bridge.send("accel_y", accel.getAccelerationY());
      bridge.send("accel_z", accel.getAccelerationZ());
    }
  }
}
