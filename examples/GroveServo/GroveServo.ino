/*
  GroveServo - Grove Servo, controlled FROM Node-RED
  on a Seeed Studio XIAO ESP32 + XIAO Expansion Board.

  Wiring: plug into the Expansion Board's *A0/D0* Grove port (or the 5V servo header).
  Node-RED: publish an angle 0..180 to  devices/esp32-demo/servo/set

  Libraries: N-R_ESP32, PubSubClient, ESP32Servo.
*/

#include <ESP32Servo.h>
#include <NodeBridge.h>

#define SERVO_PIN D0

NodeBridge bridge;
Servo servo;

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2400);   // 0.5-2.4 ms pulse range

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);

  bridge.on("servo", [](Value v) {
    int angle = constrain(v.asInt(), 0, 180);
    servo.write(angle);
  });

  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();
}
