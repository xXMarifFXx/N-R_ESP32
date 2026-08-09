/*
  GroveRGBStrip - Grove WS2813 RGB LED strip, controlled FROM Node-RED
  on a Seeed Studio XIAO ESP32 + XIAO Expansion Board.

  Wiring: plug into the Expansion Board's *A0/D0* Grove port.
  Node-RED: publish a color to  devices/esp32-demo/color/set
            as "R,G,B" (e.g. "255,0,0") or "off".

  Libraries: N-R_ESP32, PubSubClient, Adafruit NeoPixel.
*/

#include <Adafruit_NeoPixel.h>
#include <NodeBridge.h>

#define LED_PIN   D0
#define NUM_LEDS  30           // WS2813 strip: 30 LEDs/m

NodeBridge bridge;
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, strip.Color(r, g, b));
  strip.show();
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(80);
  setColor(0, 0, 0);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("192.168.1.10")
        .debug(true);

  bridge.on("color", [](Value v) {
    int r = 0, g = 0, b = 0;
    if (!strcasecmp(v.asString(), "off")) { setColor(0, 0, 0); return; }
    if (sscanf(v.asString(), "%d,%d,%d", &r, &g, &b) == 3) {
      setColor(constrain(r, 0, 255), constrain(g, 0, 255), constrain(b, 0, 255));
    }
  });

  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();
}
