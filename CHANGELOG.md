# Changelog

All notable changes to **N-R_ESP32** are documented here.
Format follows [Keep a Changelog](https://keepachangelog.com/); versions are semver.

## [1.0.4] - 2026-08-09
### Changed (performance / efficiency)
- `_handleMessage` resolves the `<key>` segment **once** per incoming command
  (new pure `nbparse::matchSetTopic`) instead of rebuilding the expected topic
  once *per subscription* — fewer `snprintf` calls per command.
- Command value is now extracted **in place** (`nbparse::extractValueInPlace`),
  dropping the second ~512-byte stack buffer in the MQTT callback (~halves that
  callback's peak stack; static flash/RAM essentially unchanged).
- `loop()` reuses the WiFi/MQTT state it already read for the connect/disconnect
  edge check instead of recomputing `connected()`.
### Added
- `MosquittoTLS` example: connect to a self-hosted Mosquitto/VPS broker with full
  certificate validation (Let's Encrypt / ISRG Root X1) + NTP time sync.
### Tests
- Host tests for `matchSetTopic` (topic routing) added; parser suite unchanged and green.

## [1.0.3] - 2026-08-08
### Fixed
- `extractValue` now matches `"value"` only as a JSON **key** (followed by `:`), so a
  `"value"` appearing as someone else's string value no longer false-matches.
- `Value::asBool` uses strict exact-match of `1/true/on/yes` (case-insensitive); inputs
  like `"10"` or `"1.5"` are now correctly `false` instead of `true`.
### Changed
- `begin()` logs a warning if a second `NodeBridge` instance is started (previously silent).
- `depends=PubSubClient (>=2.8)`; README documents the tested version (2.8.0) and that
  publishing inside an `on()` handler is safe (payload is copied before dispatch).
### Tests
- Added parser cases for the `"value"`-substring fix (now 15 `extractValue` cases).

## [1.0.2] - 2026-08-08
### Changed
- **Reliability:** bound the blocking MQTT connect with `setSocketTimeout(5)` so an
  unreachable broker no longer stalls `loop()` (and the user's algorithm) for the full
  default socket timeout on every retry.
### Security
- **TLS:** `secure()` without a root CA now logs an explicit warning that the server
  certificate is **not** validated; documentation steers production use toward
  `secure(rootCA)` + NTP time sync.
### Docs / tests
- Documented that config strings (`wifi/broker/login/root/begin`) are stored by pointer
  and must outlive the bridge.
- Added host-side golden tests for the payload parser (`test/`).
- Added this changelog; first software audit recorded (`BACKLOG.md`, `.audit/`).

## [1.0.1] - 2026-08-06
### Added
- HiveMQ Cloud-ready Node-RED example flow (TLS/8883) and a Dashboard 2.0 UI
  (temperature gauge + chart, presence, LED toggle, threshold slider, button/alarm).

## [1.0.0] - 2026-08-06
### Added
- Initial release. `NodeBridge` class wrapping WiFi + MQTT connect/reconnect, topic
  namespacing, JSON telemetry, forgiving command parsing, retained online/offline
  presence (LWT), optional heartbeat. TLS + username/password for cloud brokers
  (HiveMQ Cloud). Only dependency: PubSubClient. Four examples.

[1.0.4]: https://github.com/xXMarifFXx/N-R_ESP32/compare/1.0.3...1.0.4
[1.0.3]: https://github.com/xXMarifFXx/N-R_ESP32/compare/1.0.2...1.0.3
[1.0.2]: https://github.com/xXMarifFXx/N-R_ESP32/compare/1.0.1...1.0.2
[1.0.1]: https://github.com/xXMarifFXx/N-R_ESP32/compare/1.0.0...1.0.1
[1.0.0]: https://github.com/xXMarifFXx/N-R_ESP32/releases/tag/1.0.0
