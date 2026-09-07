# N-R_ESP32 — findings tracker

Re-audited: 2026-09-07 (commit `063baaf`) — **library verdict: GO, no P0/P1.**
Previous full system audit: 2026-08-13 (commit `0be0a166`).

The 2026-08-13 "NO-GO" was a *system* verdict driven by the mqtt-portal, not the library; its
library P1 is resolved and the two P2s marked open below were already fixed in code (references
were stale). v1.2.0 then added Arduino UNO R4 WiFi support (compiles clean on ESP32 and
`arduino:renesas_uno:unor4wifi`).

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
- [x] **Outage responsiveness.** Resolved: exponential backoff (3s→30s) + per-board jitter and `setSocketTimeout(3)` are implemented (`src/NodeBridge.cpp` `_ensureMqtt`). Only the one-time ~10s WiFi wait in `begin()` remains (bounded; loop() stays non-blocking) — tracked as P3.
- [x] **Presence semantics.** Documented one portal account/device name per physical board; unique MQTT client IDs still prevent client collisions.
- [x] **Documentation drift.** Resolved: `docs/TEACHING.md` now states client IDs are unique per board (chip ID appended since v1.1.2) while topic/presence names may still be shared.
- [x] **Fresh compile CI configured.** GitHub Actions now runs strict host parser tests and
  compiles BasicTelemetry + MariffbPortal on generic ESP32, XIAO ESP32-C3 and XIAO ESP32-S3
  using ESP32 core 3.3.11 and PubSubClient 2.8.0. Keep this checked only if the first live
  workflow run passes; local `arduino-cli` remains unavailable.

## Open (P3)

- [ ] **UNO R4 custom CA.** `WiFiSSLClient` verifies only against the radio module's built-in CA
  bundle; a private/custom root CA cannot be installed per-sketch. Fine for Let's Encrypt
  (`mqtt.mariffb.my`); documented in README + code. No action unless a private CA broker is added.
- [ ] **begin() WiFi wait.** One-time synchronous ~10s WiFi connect wait in `begin()`; bounded,
  and `loop()` is non-blocking. Consider a non-blocking begin() in a future major version.

## Verified in this audit (2026-09-07)

- Dual-arch build: `NodeBridge` compiles clean for `esp32:esp32:esp32` and
  `arduino:renesas_uno:unor4wifi` (BasicTelemetry, MariffbPortal, UnoR4Telemetry), verified locally.
- Portable unique client ID: efuse MAC (ESP32) / WiFi MAC (UNO R4), built once WiFi is up.
- TLS behaviour correct per board (ESP32 unchanged; UNO R4 verified via module bundle).

## Verified in the 2026-08-13 audit

- Unique MQTT client ID per physical board is implemented at `src/NodeBridge.cpp:54-58`; Node-RED/portal sessions with different client IDs do not disconnect one another.
- `loop()` calls PubSubClient maintenance while connected and automatically retries Wi-Fi/MQTT.
- MQTT keepalive defaults to 60 seconds; it is not a connection lifetime and cannot compensate for a blocked user loop.
- Host parser/topic tests pass under `g++ -std=c++11 -Wall -Wextra -pedantic`.
- Static buffers are bounded and telemetry values are JSON-escaped; the device field still needs escaping.
- No committed real credential was found in tracked examples/files.
