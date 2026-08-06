# Design notes — N-R_ESP32

Working notes for the library. Kept in-repo so the reasoning travels with the code.

## Goal

One class that lets an ESP32 talk to Node-RED (via any MQTT broker) with the least
possible boilerplate. The user writes their algorithm; the library handles WiFi,
MQTT, reconnection, topic layout, encoding, and presence.

## Decisions

| Decision | Choice | Why |
|----------|--------|-----|
| MQTT transport | **PubSubClient** | Smallest, most widely compatible client; works across every ESP32 core version. AsyncMqttClient is faster but drags in AsyncTCP and is fussier across cores. |
| Payload format | **JSON out, forgiving in** | Node-RED parses JSON natively. Outgoing is a fixed flat shape we control; incoming accepts raw scalars or `{"value":..}`. |
| JSON handling | **hand-rolled, no ArduinoJson** | Outgoing JSON is flat and fixed → `snprintf` is safe and tiny. Incoming only needs one scalar → a small extractor. Removing ArduinoJson means zero v6/v7 API drift and one fewer install. Cost: not for arbitrary nested command payloads (fine for this use case). |
| Automation level | **Full auto** | Auto WiFi + MQTT reconnect, auto topic namespacing, retained online/offline presence via Last Will, optional heartbeat. |
| Instance model | **single global instance** | PubSubClient's callback is a C function pointer; a static `_self` trampoline routes it to the instance. Documented as one-instance. |

## Topic scheme

```
devices/<device>/<key>          telemetry   ESP32 -> Node-RED   {"value":..,"device":..,"ts":..}
devices/<device>/<key>/set      command     Node-RED -> ESP32   raw scalar or {"value":..}
devices/<device>/status         presence    retained            online | offline (LWT)
devices/<device>/heartbeat      health      retained            {"rssi":..,"uptime_s":..,"ip":..}
```

`/set` suffix separates commands from telemetry so an ESP32 never re-consumes its own
published data. `devices` root is overridable via `.root()`.

## API surface

Chainable config (`wifi/broker/login/root/heartbeat/debug/onConnect/onDisconnect/on`)
→ `begin(name)` → `loop()`. Telemetry via overloaded `send()`. Commands via `on(key, handler)`
delivering a typed `Value`.

## Reconnection model

- `begin()` blocks up to 10 s for the first WiFi join, then tries MQTT once. Non-fatal.
- `loop()` re-checks WiFi (nudge every 5 s), retries MQTT every 3 s, drives `_mqtt.loop()`,
  emits heartbeat, and fires connect/disconnect edges exactly once.

## TLS (added for cloud brokers)

`.secure()` switches the transport from `WiFiClient` to `WiFiClientSecure` and is
required by HiveMQ Cloud and most managed brokers (port 8883).

- `begin()` selects the client via `_mqtt.setClient()` based on `_tls`.
- `broker(host, port=0)` — port 0 means "auto": resolved to 8883 (TLS) or 1883 (plain)
  in `begin()`, so users usually omit the port entirely.
- `secure()` with no arg calls `setInsecure()` — encrypted but unvalidated (quick start).
  `secure(rootCA)` calls `setCACert()` for real validation; requires a roughly-correct
  clock (NTP) because TLS checks cert expiry.
- Combine with `.login(user, pass)` for the username/password cloud brokers expect.

## Possible future work

- Optional QoS-1 publishes / persistent session.
- Auto-discovery payload so Node-RED can enumerate devices.
- Batching multiple keys into one message.
- Compile-time check example under CI (arduino-cli) — see build note below.

## Build / smoke test

No ESP32 toolchain is assumed on the dev machine. To compile-check locally:

```bash
arduino-cli core install esp32:esp32
arduino-cli lib install PubSubClient
arduino-cli compile --fqbn esp32:esp32:esp32 examples/BasicTelemetry
```
