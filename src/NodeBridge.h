/*
  NodeBridge.h  -  Dead-simple ESP32 <-> MQTT <-> Node-RED bridge

  Part of the N-R_ESP32 library.

  Hides WiFi setup, MQTT connect/reconnect, topic naming, JSON encoding and
  presence (online/offline) so your sketch only contains your algorithm.

  Works on any ESP32 Arduino core version.

  Dependency (install via Library Manager):
    - PubSubClient  by Nick O'Leary

  License: MIT
*/

#ifndef NODEBRIDGE_H
#define NODEBRIDGE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#ifndef NODEBRIDGE_MAX_SUBS
#define NODEBRIDGE_MAX_SUBS 16      // max number of on(...) command handlers
#endif

#ifndef NODEBRIDGE_BUFFER_SIZE
#define NODEBRIDGE_BUFFER_SIZE 512  // MQTT payload buffer (bytes)
#endif

// ----------------------------------------------------------------------------
// Value  -  a tiny typed view of an incoming command payload.
// You get one of these in every on(...) handler and read it however you like:
//     bridge.on("led", [](Value v){ digitalWrite(2, v.asBool()); });
// ----------------------------------------------------------------------------
class Value {
public:
  explicit Value(const char* raw) : _raw(raw ? raw : "") {}

  const char* asString() const { return _raw; }
  float       asFloat()  const { return atof(_raw); }
  int         asInt()    const { return atoi(_raw); }
  long        asLong()   const { return atol(_raw); }
  bool asBool() const {
    return  !strcasecmp(_raw, "1")    ||
            !strcasecmp(_raw, "true") ||
            !strcasecmp(_raw, "on")   ||
            !strcasecmp(_raw, "yes");
  }

  operator const char*() const { return _raw; }   // implicit: String s = value;

private:
  const char* _raw;
};

typedef void (*CommandHandler)(Value value);
typedef void (*EventHandler)();

// ----------------------------------------------------------------------------
// NodeBridge  -  the whole library. Create ONE instance globally.
// ----------------------------------------------------------------------------
class NodeBridge {
public:
  NodeBridge();

  // ---- configuration (chainable; call before begin) ---------------------
  NodeBridge& wifi(const char* ssid, const char* password);
  NodeBridge& broker(const char* host, uint16_t port = 0);     // 0 = auto (8883 TLS / 1883 plain)
  NodeBridge& login(const char* user, const char* password);   // optional MQTT auth

  // Enable TLS (required by HiveMQ Cloud and most cloud brokers, port 8883).
  //   secure()          -> encrypted, server cert NOT validated (quick start)
  //   secure(rootCA)     -> encrypted + validated against the PEM root CA you pass
  NodeBridge& secure(const char* rootCA = nullptr);
  NodeBridge& root(const char* rootTopic);                     // default "devices"
  NodeBridge& heartbeat(unsigned long intervalMs);             // 0 = off (default)
  NodeBridge& keepAlive(uint16_t seconds);                     // MQTT keep-alive (default 60s)
  NodeBridge& debug(bool on = true);                           // log to Serial

  NodeBridge& onConnect(EventHandler cb);
  NodeBridge& onDisconnect(EventHandler cb);

  // Subscribe to a command from Node-RED. Handler fires when Node-RED
  // publishes to  <root>/<device>/<key>/set
  NodeBridge& on(const char* key, CommandHandler handler);

  // ---- lifecycle --------------------------------------------------------
  bool begin(const char* deviceName);   // connect WiFi + MQTT (non-fatal if down)
  void loop();                          // call every loop() iteration

  // ---- publishing telemetry to Node-RED ---------------------------------
  // Publishes JSON {"value":<v>,"device":"<name>","ts":<ms>} to
  //   <root>/<device>/<key>
  bool send(const char* key, float value);
  bool send(const char* key, double value);
  bool send(const char* key, int value);
  bool send(const char* key, long value);
  bool send(const char* key, bool value);
  bool send(const char* key, const char* value);
  bool send(const char* key, const String& value);

  // Escape hatch: publish an exact payload to an exact topic.
  bool sendRaw(const char* topic, const char* payload, bool retained = false);

  // ---- status -----------------------------------------------------------
  bool connected();
  const char* deviceName() const { return _device; }

private:
  struct Sub { const char* key; CommandHandler handler; };

  // wiring
  WiFiClient       _net;         // plain transport
  WiFiClientSecure _netSecure;   // TLS transport
  PubSubClient     _mqtt;

  // config
  const char* _ssid   = nullptr;
  const char* _pass   = nullptr;
  const char* _host   = nullptr;
  uint16_t    _port   = 0;        // 0 = auto-pick from _tls in begin()
  const char* _user   = nullptr;
  const char* _mpass  = nullptr;
  const char* _root   = "devices";
  const char* _device = "esp32";
  char        _clientId[48] = {0};   // <device>-<chipid>, unique per board
  bool          _tls    = false;
  const char*   _caCert = nullptr;
  bool          _debug         = false;
  uint16_t      _keepAlive     = 60;   // seconds; higher = tolerates longer gaps before the broker drops us
  unsigned long _hbInterval    = 0;
  unsigned long _lastHb        = 0;
  unsigned long _lastReconnect = 0;

  EventHandler _onConnect    = nullptr;
  EventHandler _onDisconnect = nullptr;
  bool         _wasConnected = false;

  Sub _subs[NODEBRIDGE_MAX_SUBS];
  uint8_t _subCount = 0;

  // internals
  static NodeBridge* _self;                     // single-instance trampoline
  static void _trampoline(char* topic, uint8_t* payload, unsigned int len);
  void   _handleMessage(char* topic, uint8_t* payload, unsigned int len);
  bool   _ensureWifi();
  bool   _ensureMqtt();
  void   _resubscribe();
  void   _topic(char* out, size_t n, const char* key, const char* suffix = nullptr);
  bool   _publishValue(const char* key, const char* jsonValue);
  void   _log(const char* msg);
};

#endif // NODEBRIDGE_H
