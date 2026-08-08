# N-R_ESP32 — Findings tracker

Audit baseline: `33af29b` · 2026-08-08 · full run. Verdict: **GO** (no P0/P1).
Profile: Arduino/ESP32 **library** · C++ (ESP32 core) + PubSubClient · handles WiFi/MQTT
credentials & telemetry (no persistent PII) · exposure: published artifact, device↔broker
over network (TLS optional) · distribution: Arduino Library Manager candidate.

Severity: P0 blocker · P1 high · P2 medium · P3 low. Tick when resolved.

## P2 — resolved in 1.0.2
- [x] **[Security] TLS certificate validation off by default.** MITIGATED (1.0.2): `secure()` without
  a CA now logs an explicit warning; README steers production to `secure(rootCA)` + NTP. Residual:
  the insecure path still exists by design (quick start) — acceptable for a library, use CA in prod.
- [x] **[Reliability] Blocking reconnect stalls loop().** MITIGATED (1.0.2): `setSocketTimeout(5)` in
  `begin()` bounds the synchronous connect/read to ~5 s instead of the ~15 s+ default. Residual:
  still synchronous — truly non-blocking needs AsyncMqttClient (noted for a future major).

## P3 — schedule / polish
- [x] **[Correctness] Config stored as raw `const char*`.** DOCUMENTED (1.0.2): README "Notes/limits"
  now states config strings are stored by pointer and must outlive the bridge. `src/NodeBridge.cpp:78-90`.
- [ ] **[Correctness] Single global instance.** `_self` static trampoline; a 2nd NodeBridge routes
  all callbacks to whichever called `begin()` last. `src/NodeBridge.cpp:9,113`. Documented, not enforced.
- [ ] **[Correctness] `extractValue` can false-match `"value"` as a substring** of another string
  value; returns whole payload. `src/NodeBridge.cpp:38`. Minor; fine for intended flat payloads.
- [ ] **[Correctness] `Value::asBool` treats any string starting with `'1'` as true** ("1.5","10x").
  `src/NodeBridge.h:47`. Edge only.
- [ ] **[Supply chain] `depends=PubSubClient` unpinned** and PubSubClient is low-maintenance
  (last release 2.8, 2020). `library.properties`. Note the pinned tested version in README.
- [ ] **[Reentrancy] Publishing inside an `on()` handler** (SensorAndActuator threshold echo). The
  library copies the payload before invoking the handler (`NodeBridge.cpp:238-242`) so the library
  side is safe, but re-entrant `PubSubClient::publish()` is a known caveat — verify on hardware or
  defer the echo to `loop()`.
- [x] **[Code quality] No in-repo tests / CHANGELOG.** DONE (1.0.2): parser helpers extracted to
  `src/nb_parse.h`; host golden tests in `test/test_parse.cpp` (14 cases, all pass); `CHANGELOG.md` added.

## Verified-good (clean at 33af29b — don't re-audit unless changed)
- Memory safety: all topic/payload buffers bounded via `snprintf`/`memcpy`+cap; payloads null-terminated.
  `NodeBridge.cpp:158,196,232,236-239,264-268`.
- `millis()` rollover handled with unsigned subtraction. `NodeBridge.cpp:135,150,156,181`.
- No committed secrets (git grep clean; examples use placeholders).
- License MIT present; dependency PubSubClient is MIT — compatible.
- Compiles clean (`-Wall -Wextra`) on esp32:esp32 3.3.11 and arduino:esp32 2.0.18 (ESP32 + ESP32-S3).
- LWT online/offline presence correct. `NodeBridge.cpp:200,205`.

## Skipped dimensions (with reason)
- Data integrity (3), Data protection/compliance (4), Accessibility (7): N/A — stateless headless
  library, no persistent store, no PII ownership, no UI. Telemetry privacy is the app developer's
  responsibility (TLS is available).
