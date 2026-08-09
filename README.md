# N-R_ESP32

A library that makes ESP32 ⟷ MQTT broker ⟷ Node-RED communication **simple and clean**,
so you can focus on your algorithm instead of the plumbing.

It wraps WiFi, MQTT connect/reconnect, topic naming, JSON encoding and
online/offline presence behind one small class — `NodeBridge`.

```cpp
#include <NodeBridge.h>
NodeBridge bridge;

void setup() {
  bridge.wifi("SSID", "password")
        .broker("192.168.1.10");
  bridge.on("led", [](Value v){ digitalWrite(2, v.asBool()); });  // from Node-RED
  bridge.begin("esp32-livingroom");
}

void loop() {
  bridge.loop();                        // keeps everything alive + auto-reconnects
  bridge.send("temperature", 24.5);     // to Node-RED
}
```

Works on **any ESP32 Arduino core version**. Only one dependency: **PubSubClient**.

---

## Install

1. **Arduino IDE** → *Tools → Manage Libraries* → install **PubSubClient** (by Nick O'Leary).
2. Add this library:
   - *Sketch → Include Library → Add .ZIP Library…* and pick a ZIP of this repo, **or**
   - clone into your `Arduino/libraries/` folder:
     ```bash
     git clone https://github.com/xXMarifFXx/N-R_ESP32.git
     ```
3. Restart the IDE. Examples appear under *File → Examples → N-R_ESP32*.

---

## How it talks to Node-RED

The library uses a consistent, tidy topic scheme so your Node-RED flows stay clean:

| Direction | Topic | Payload |
|-----------|-------|---------|
| ESP32 → Node-RED (telemetry) | `devices/<device>/<key>` | `{"value":<v>,"device":"<device>","ts":<ms>}` |
| Node-RED → ESP32 (command)   | `devices/<device>/<key>/set` | `on` / `24.5` / `{"value":"on"}` |
| Presence (auto, retained)    | `devices/<device>/status` | `online` / `offline` |
| Heartbeat (optional)         | `devices/<device>/heartbeat` | `{"rssi":-58,"uptime_s":42,"ip":"..."}` |

- **Telemetry** is JSON so a Node-RED `mqtt-in` node (datatype *auto/JSON*) gives you
  `msg.payload.value` directly.
- **Commands** are forgiving: send a raw value (`on`, `1`, `24.5`) or JSON with a
  `value` field — the library extracts the scalar either way, and `Value` converts it
  to `bool` / `int` / `float` / `String`.
- **Presence** uses MQTT's Last Will, so if the ESP32 loses power the broker marks it
  `offline` for you.

`devices` is the default namespace root; change it with `.root("myfleet")`.

Import [`node-red/example-flow.json`](node-red/example-flow.json) to get a matching
flow: telemetry debug, LED test buttons, threshold setter, **and a live Dashboard**
(temperature gauge + chart, online/offline indicator, button + alarm status, LED
toggle, threshold slider). The button/alarm indicators light up when you run the
**SensorAndActuator** sketch.

The dashboard uses **Dashboard 2.0** (`@flowfuse/node-red-dashboard`). Install it once via
*Menu → Manage palette → Install → `@flowfuse/node-red-dashboard`*, then **Deploy** and open
`http://<node-red-host>:1880/dashboard`.

---

## API

**Configuration** (chainable, call before `begin`):

| Method | Purpose |
|--------|---------|
| `wifi(ssid, pass)` | WiFi credentials |
| `broker(host, port=0)` | MQTT broker address (port 0 = auto: 8883 TLS / 1883 plain) |
| `login(user, pass)` | MQTT username/password (if broker requires) |
| `secure(rootCA=nullptr)` | enable TLS — required by HiveMQ Cloud & most cloud brokers |
| `root(name)` | topic namespace root (default `"devices"`) |
| `heartbeat(ms)` | publish rssi/uptime every `ms` (0 = off) |
| `debug(true)` | print status to `Serial` |
| `onConnect(fn)` / `onDisconnect(fn)` | link/unlink callbacks |
| `on(key, handler)` | subscribe to a command from Node-RED |

**Lifecycle:**

| Method | Purpose |
|--------|---------|
| `begin(deviceName)` | connect WiFi + MQTT (non-blocking after first try) |
| `loop()` | call every iteration; maintains + auto-reconnects |
| `connected()` | `true` when WiFi **and** MQTT are up |

**Send telemetry** — overloaded for `float`, `double`, `int`, `long`, `bool`,
`const char*`, `String`:

```cpp
bridge.send("temperature", 24.5);
bridge.send("button", true);
bridge.send("status", "running");
bridge.sendRaw("custom/topic", "hello", /*retained=*/false);   // escape hatch
```

**Receive commands** — the handler gets a `Value` you read as any type:

```cpp
bridge.on("threshold", [](Value v){ setpoint = v.asFloat(); });
// v.asBool(), v.asInt(), v.asLong(), v.asString(), or use it directly as const char*
```

---

## Cloud brokers (HiveMQ Cloud, etc.)

Cloud MQTT brokers require **TLS + username/password**. Add `.secure()` and `.login()`:

```cpp
bridge.wifi("SSID", "pass")
      .broker("xxxxxxxx.s1.eu.hivemq.cloud")   // port auto-defaults to 8883
      .secure()                                // TLS on
      .login("hivemq-user", "hivemq-pass");
bridge.begin("esp32-demo");
```

Point your Node-RED `mqtt-broker` node at the **same** cluster URL / port 8883, tick
**Use TLS**, and enter the same username/password — both ends meet in the cloud.

- `.secure()` encrypts but does **not** verify the broker's certificate (fine to start).
- `.secure(rootCA)` verifies against a PEM root CA (HiveMQ Cloud uses Let's Encrypt /
  ISRG Root X1). With validation on, sync the ESP32 clock via NTP first so cert-expiry
  checks pass. See the **HiveMQCloud** example for the full pattern.

## Examples

- **BasicTelemetry** — publish a reading every 2 s.
- **ReceiveCommands** — toggle the onboard LED from Node-RED.
- **SensorAndActuator** — the full picture: send data, receive commands, heartbeat, presence.
- **HiveMQCloud** — connect to HiveMQ Cloud over TLS with username/password.
- **MosquittoTLS** — connect to your own broker with full certificate validation (ISRG Root X1) + NTP.

### Seeed Studio XIAO + Grove Starter Kit

Ready-to-run examples for every sensor/actuator in the **Grove Starter Kit for XIAO**
(SKU 110010044), on the **XIAO Expansion Board**. All verified compiling on
`XIAO_ESP32C3` and `XIAO_ESP32S3`. Sensors `send()`; actuators subscribe via `on()`.

| Example | Grove module | Port | MQTT |
|---------|--------------|------|------|
| **GroveTempHumidity** | Temp & Humidity V2.0 (DHT20) | I²C | → `temperature`, `humidity` |
| **GroveAccelerometer** | 3-Axis Accelerometer (LIS3DHTR) | I²C | → `accel_x/y/z` |
| **GroveLightSensor** | Light Sensor v1.2 | A0 | → `light` |
| **GroveRotaryAngle** | Rotary Angle Sensor | A0 | → `rotary` |
| **GrovePIRMotion** | Mini PIR Motion | D0 | → `motion` |
| **GroveIRReceiver** | IR Receiver + 20-key remote | D0 | → `ir_key` |
| **GroveServo** | Servo | D0 | ← `servo/set` (0–180) |
| **GroveRGBStrip** | WS2813 RGB strip | D0 | ← `color/set` ("R,G,B"/"off") |
| **ExpansionBuzzer** | onboard buzzer | D3 | ← `buzzer/set` ("on"/"off"/Hz) |
| **ExpansionOLED** | onboard OLED (SSD1306) | I²C | ← `text/set` (shows text) |

Each pulls one sensor library (DHT20, LIS3DHTR, ESP32Servo, Adafruit NeoPixel, IRremote,
or U8g2) — the IDE offers to install it. See [docs/XIAO_GROVE.md](docs/XIAO_GROVE.md) for wiring.

---

## Notes / limits

- Create **one** `NodeBridge` instance (it registers a single MQTT callback). A second
  instance logs a warning and won't receive commands.
- Publishing from inside an `on()` handler is safe — the incoming payload is copied
  before your handler runs, so `send()` inside a handler won't corrupt it.
- Tested with **PubSubClient 2.8.0**.
- Up to 16 `on()` handlers by default — raise with
  `#define NODEBRIDGE_MAX_SUBS 32` before `#include`.
- Max payload 512 bytes by default — raise with `#define NODEBRIDGE_BUFFER_SIZE 1024`.
- `String` keys/values are fine; keys passed to `on()`/`send()` should be string
  literals or otherwise stay in scope.
- **Config strings must outlive the bridge.** `wifi()`, `broker()`, `login()`, `root()`
  and `begin()` store the *pointer* you pass, not a copy — use string literals or
  variables that stay in scope (don't pass a temporary `String::c_str()`).
- **For production TLS, validate the certificate.** `secure()` with no argument does
  *not* verify the broker (MITM-able); pass `secure(rootCA)` and sync the clock via NTP.
  The library logs a warning when running unvalidated.

## Teaching a class with this?

See **[docs/TEACHING.md](docs/TEACHING.md)** — how to pre-warm the compile cache so
builds are fast in class, Windows Defender exclusions, and HiveMQ Cloud classroom
gotchas (unique device names, the free-tier connection limit, port 8883).

## License

MIT — see [LICENSE](LICENSE).
