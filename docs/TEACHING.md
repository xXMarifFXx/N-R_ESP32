# Classroom Fast-Start — N-R_ESP32

A practical checklist for teaching with this library so the **first compile
doesn't eat your class time**.

---

## Why the first build is slow (and it's not this library)

The first time you compile *anything* for an ESP32, the IDE builds the whole
**ESP32 framework** (WiFi, TLS/mbedTLS, RTOS…). Measured clean-build times:

| Sketch | Includes | Time (fast PC) |
|--------|----------|----------------|
| Blink  | nothing — no WiFi, no library | ~27 s |
| WiFi, no TLS | WiFi + PubSubClient | ~33 s |
| WiFi + TLS (HiveMQ) | WiFiClientSecure + PubSubClient | ~37 s |

A blank **blink** sketch already takes ~27 s — that's the framework, which
*every* ESP32 sketch pays. **N-R_ESP32 adds almost nothing.** On a cold Windows
laptop (first build ever + antivirus scanning), the same work can take
**10–15 minutes** — but only **once**.

**The key fact:** after the first build, the framework is **cached**. Every build
afterwards — including switching between examples — drops to **~30–60 seconds**.

So the fix isn't a lighter library (we're already at the framework floor); it's
**pre-warming the cache before class**.

---

## Before class — do this on every laptop (once)

1. **Install everything up front** (so nothing downloads mid-class):
   - ESP32 core: *Tools → Board → Boards Manager* → **esp32** by Espressif
   - *Tools → Manage Libraries* → **PubSubClient**
   - **N-R_ESP32** (Library Manager, or *Sketch → Include Library → Add .ZIP*)
2. **Select the exact board** you'll use (e.g. *ESP32 Dev Module*) — and **don't change it later**.
3. **Pre-warm the cache:** open *File → Examples → N-R_ESP32 → HiveMQCloud* and hit
   **Verify (✓)**. Let the first build finish (this is the slow one — ~10–15 min).
4. **Windows only — add Defender exclusions** (big speedup; Defender scans thousands
   of build files). *Virus & threat protection → Manage settings → Exclusions → Add folder*:
   - `%LOCALAPPDATA%\Arduino15`
   - `%USERPROFILE%\AppData\Local\Temp`

After step 3, every build for the rest of the day is **~30–60 s**.

> Lab PCs? Do this the night before. Students' own laptops? Have them run steps 1–4
> as a pre-lab, or during your intro (see below).

---

## During class

- **Warm-up while you talk:** have students press **Verify** on the starter sketch
  during your introduction/slides. It compiles in the background so it's fast when
  they start coding.
- **Never switch the board** (Tools → Board). Changing it invalidates the cache and
  forces the full first-build again.
- **Keep the IDE open** — restarting can cost you the in-memory cache.

---

## HiveMQ Cloud in a classroom — read this

A shared cloud broker has two gotchas that will bite a class:

1. **Every device needs a UNIQUE name.** The ESP32 uses its device name as its MQTT
   **client ID**. If two students both use `begin("esp32-demo")`, they'll **kick each
   other offline** (same client ID). Give each student a unique name:
   ```cpp
   bridge.begin("esp32-aisha");   // or esp32-01, esp32-02, ...
   ```
   Then update their Node-RED flow topics to match (`devices/esp32-aisha/#`, etc.).

2. **Free-tier connection limit.** HiveMQ Cloud's free *Serverless* plan allows about
   **25 concurrent connections per cluster**. Each ESP32 **and** each Node-RED counts
   as one. A class of 15 (ESP32 + Node-RED each = 30 connections) exceeds one free cluster.
   - **Recommended for class:** have **each student create their own free HiveMQ Cloud
     cluster** (separate account → own 25-connection budget, and no name collisions to
     worry about).
   - Or use one shared paid cluster with unique names as above.

3. **Internet required.** HiveMQ Cloud needs working internet and outbound **TCP 8883**.
   Some school/campus networks block non-standard ports — **test the classroom network
   beforehand**. If it's blocked, fall back to a local Mosquitto broker (set port 1883
   and remove `.secure()`).

---

## Per-student checklist

- [ ] ESP32 core + PubSubClient + N-R_ESP32 installed
- [ ] Board selected — and locked (don't change it)
- [ ] Pre-warmed: one sketch Verified successfully once
- [ ] (Windows) Defender folder exclusions added
- [ ] Unique device name chosen (`esp32-<name>`)
- [ ] HiveMQ cluster URL + username + password ready
- [ ] Node-RED flow topics updated to match the device name
- [ ] Classroom network allows outbound TCP 8883
