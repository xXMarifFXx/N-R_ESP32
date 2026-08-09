/*
  GroveIRReceiver - Grove Infrared Receiver + the kit's 20-key mini remote
  on a Seeed Studio XIAO ESP32 + XIAO Expansion Board.

  Wiring: plug into the Expansion Board's *A0/D0* Grove port.
  Publishes:  devices/esp32-demo/ir_key   (the numeric command of the button pressed)

  Libraries: N-R_ESP32, PubSubClient, IRremote (by Armin Joachimsmeyer).
*/

#define DECODE_NEC              // the 20-key mini remote uses the NEC protocol
#include <IRremote.hpp>
#include <NodeBridge.h>

#define IR_PIN D0

NodeBridge bridge;

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_PIN);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);
  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();

  if (IrReceiver.decode()) {
    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {  // ignore held-key repeats
      bridge.send("ir_key", (int)IrReceiver.decodedIRData.command);
    }
    IrReceiver.resume();     // ready for the next press
  }
}
