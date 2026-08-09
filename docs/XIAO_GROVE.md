# XIAO + Grove Starter Kit — wiring & examples

Companion notes for the `Grove*` / `Expansion*` examples. Hardware: a **Seeed Studio
XIAO ESP32** board (C3 / S3 / C6) seated on the **XIAO Expansion Board**, with modules
from the **Grove Starter Kit for XIAO** (SKU 110010044).

## Boards these run on
Only the Wi-Fi XIAO variants run this library: **XIAO ESP32-C3, ESP32-S3, ESP32-C6**
(FQBNs `esp32:esp32:XIAO_ESP32C3` / `XIAO_ESP32S3` / `XIAO_ESP32C6`). The SAMD21,
RP2040/RP2350 and nRF52840 XIAO boards have no Wi-Fi and are not supported.

## Pin cheat-sheet (XIAO + Expansion Board)
- On XIAO, physical pin *N* is both `D`*N* and `A`*N* — the **same pin**. Use `D3` (not
  `A3`); the analog alias isn't defined on every variant.
- **I²C** (`Wire`) is on **D4 (SDA) / D5 (SCL)** by default — just call `Wire.begin()`.
- Expansion Board built-ins: **OLED SSD1306** @ I²C `0x3C` · **buzzer** on **D3** ·
  user **button** on **D1** · microSD CS on **D2** · **RTC PCF8563** on I²C.
- The Expansion Board's Grove ports: one **I²C**, one **UART**, one **analog/digital (A0/D0)**.
  Single-pin Grove modules (light, rotary, PIR, IR, servo, RGB strip) share the **A0/D0**
  port — use one at a time. I²C modules (temp/humidity, accelerometer, OLED) share the bus.

## Module → example → library
| Module | Example | Library (Library Manager name) | Interface |
|--------|---------|-------------------------------|-----------|
| Temp & Humidity V2.0 (DHT20) | GroveTempHumidity | `DHT20` | I²C |
| 3-Axis Accel (LIS3DHTR) | GroveAccelerometer | `Grove-3-Axis-Digital-Accelerometer-2g-to-16g-LIS3DHTR` | I²C |
| Light Sensor v1.2 | GroveLightSensor | — (analogRead) | A0 |
| Rotary Angle | GroveRotaryAngle | — (analogRead) | A0 |
| Mini PIR Motion | GrovePIRMotion | — (digitalRead) | D0 |
| IR Receiver + remote | GroveIRReceiver | `IRremote` | D0 |
| Servo | GroveServo | `ESP32Servo` | D0 |
| WS2813 RGB strip | GroveRGBStrip | `Adafruit NeoPixel` | D0 |
| Onboard buzzer | ExpansionBuzzer | — (tone) | D3 |
| Onboard OLED | ExpansionOLED | `U8g2` | I²C |

## Node-RED
The examples publish under `devices/esp32-demo/…` and subscribe on `…/set`, so they line
up with the dashboard flow in `node-red/example-flow.json`. The temp/humidity example
drives the existing temperature gauge out of the box; add `mqtt-in` nodes on
`accel_x`, `light`, `rotary`, `motion`, `ir_key` for the rest, and `mqtt-out` nodes on
`servo/set`, `color/set`, `buzzer/set`, `text/set` to drive the actuators.

## Verified
All ten examples compile clean on `XIAO_ESP32C3` and `XIAO_ESP32S3` (esp32 core 3.3.11).
Runtime behaviour needs the physical modules — flash one, open Serial at 115200, and watch
the `[NodeBridge]` logs.
