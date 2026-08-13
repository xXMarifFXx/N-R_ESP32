# N-R_ESP32 — findings tracker

Full system audit: 2026-08-13

Commit: `0be0a1661392b54c8b2213cf4e7a3d2f50c86e9c`
System verdict: **NO-GO**. The complete cross-repository report is in the mqtt-portal repository's `AUDIT.md`.

## P1

- [x] **Portal namespace contract.** Fixed for 1.1.3: `MariffbPortal` uses one
  `MQTT_USERNAME` for both `login()` and `begin()`, `MosquittoTLS` follows the same rule for
  username-scoped brokers, and the portal generator has a regression test. The Arduino
  Library Manager release and physical ESP32 end-to-end check remain release gates.

## P2

- [x] **TLS verification for mqtt.mariffb.my.** `MariffbPortal` and the portal-generated sketch
  use the bundled ISRG Root X1 through `secure(NODEBRIDGE_ISRG_ROOT_X1)`. The generic insecure
  quick-start API remains an accepted, documented option for other brokers.
- [x] **Validate/copy identifiers.** Device/root/key segments are bounded, restricted to safe characters, and copied into library-owned storage.
- [ ] **Outage responsiveness.** A failed synchronous MQTT connection can block for 5 seconds and retry after 3 seconds. Add exponential backoff/jitter; evaluate nonblocking transport for v2. `src/NodeBridge.cpp:81,100-112`.
- [x] **Presence semantics.** Documented one portal account/device name per physical board; unique MQTT client IDs still prevent client collisions.
- [ ] **Documentation drift.** `docs/TEACHING.md` still says boards sharing `begin("esp32-demo")` collide, although v1.1.2 appends a chip ID. Update it to explain that MQTT client IDs are unique but topic/presence names may still be shared.
- [x] **Fresh compile CI configured.** GitHub Actions now runs strict host parser tests and
  compiles BasicTelemetry + MariffbPortal on generic ESP32, XIAO ESP32-C3 and XIAO ESP32-S3
  using ESP32 core 3.3.11 and PubSubClient 2.8.0. Keep this checked only if the first live
  workflow run passes; local `arduino-cli` remains unavailable.

## Verified in this audit

- Unique MQTT client ID per physical board is implemented at `src/NodeBridge.cpp:54-58`; Node-RED/portal sessions with different client IDs do not disconnect one another.
- `loop()` calls PubSubClient maintenance while connected and automatically retries Wi-Fi/MQTT.
- MQTT keepalive defaults to 60 seconds; it is not a connection lifetime and cannot compensate for a blocked user loop.
- Host parser/topic tests pass under `g++ -std=c++11 -Wall -Wextra -pedantic`.
- Static buffers are bounded and telemetry values are JSON-escaped; the device field still needs escaping.
- No committed real credential was found in tracked examples/files.
