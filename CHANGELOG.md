# Changelog

All notable changes to **N-R_ESP32** are documented here.
Format follows [Keep a Changelog](https://keepachangelog.com/); versions are semver.

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

[1.0.2]: https://github.com/xXMarifFXx/N-R_ESP32/compare/1.0.1...1.0.2
[1.0.1]: https://github.com/xXMarifFXx/N-R_ESP32/compare/1.0.0...1.0.1
[1.0.0]: https://github.com/xXMarifFXx/N-R_ESP32/releases/tag/1.0.0
