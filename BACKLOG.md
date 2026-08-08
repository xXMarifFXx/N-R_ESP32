# N-R_ESP32 — Findings tracker

Audit baseline: `33af29b` · 2026-08-08 · full run. Verdict: **GO** (no P0/P1).
Status: all P2 mitigated (1.0.2) and all P3 cleared (1.0.3). No open findings.
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
- [x] **[Correctness] Single global instance.** DETECTED (1.0.3): `begin()` now logs a warning when a
  second instance is started (`_self != this`). Multi-instance still unsupported by design (one
  PubSubClient C callback), but no longer silent.
- [x] **[Correctness] `extractValue` false-match on `"value"` substring.** FIXED (1.0.3): now matches
  `"value"` only when it is a key (followed by `:`). Covered by new tests in `test/test_parse.cpp`.
- [x] **[Correctness] `Value::asBool` leading-`'1'`.** FIXED (1.0.3): strict exact-match of
  `1/true/on/yes` (case-insensitive); `"10"`, `"1.5"` now correctly false. `src/NodeBridge.h`.
- [x] **[Supply chain] PubSubClient version.** PINNED (1.0.3): `depends=PubSubClient (>=2.8)`;
  README notes "tested with PubSubClient 2.8.0".
- [x] **[Reentrancy] Publishing inside an `on()` handler.** DOCUMENTED (1.0.3): confirmed safe (payload
  copied before dispatch, `NodeBridge.cpp`); README + example comment state it explicitly.
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
