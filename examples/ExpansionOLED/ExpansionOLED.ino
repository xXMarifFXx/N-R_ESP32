/*
  ExpansionOLED - the 0.96" OLED (SSD1306, I2C 0x3C) built into the XIAO
  Expansion Board. Shows text pushed FROM Node-RED, plus live connection status.

  Node-RED: publish any text to  devices/esp32-demo/text/set  -> it appears on screen.

  Libraries: N-R_ESP32, PubSubClient, U8g2 (by oliver / olikraus).
*/

#include <U8g2lib.h>
#include <Wire.h>
#include <NodeBridge.h>

NodeBridge bridge;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
char line[48] = "Waiting...";

void draw(const char* status, const char* msg) {
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x12_tr);
  oled.drawStr(0, 12, status);
  oled.drawHLine(0, 16, 128);
  oled.setFont(u8g2_font_7x14B_tr);
  oled.drawStr(0, 40, msg);
  oled.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  oled.begin();
  draw("Connecting...", "");

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);

  bridge.on("text", [](Value v) {
    strncpy(line, v.asString(), sizeof(line) - 1);
    line[sizeof(line) - 1] = '\0';
    draw("esp32-demo online", line);
  });
  bridge.onConnect([]() { draw("esp32-demo online", line); });
  bridge.onDisconnect([]() { draw("reconnecting...", line); });

  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();
}
