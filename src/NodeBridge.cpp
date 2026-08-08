/*
  NodeBridge.cpp  -  implementation
  Part of the N-R_ESP32 library.  License: MIT
*/

#include "NodeBridge.h"
#include "nb_parse.h"   // pure, host-testable payload helpers (see test/)

// Single global instance pointer so PubSubClient's C-style callback can reach us.
NodeBridge* NodeBridge::_self = nullptr;

// ---------------------------------------------------------------------------
// construction / configuration
// ---------------------------------------------------------------------------

NodeBridge::NodeBridge() : _mqtt(_net) {}

NodeBridge& NodeBridge::wifi(const char* ssid, const char* password) {
  _ssid = ssid; _pass = password; return *this;
}
NodeBridge& NodeBridge::broker(const char* host, uint16_t port) {
  _host = host; _port = port; return *this;
}
NodeBridge& NodeBridge::login(const char* user, const char* password) {
  _user = user; _mpass = password; return *this;
}
NodeBridge& NodeBridge::secure(const char* rootCA) {
  _tls = true; _caCert = rootCA; return *this;
}
NodeBridge& NodeBridge::root(const char* rootTopic) { _root = rootTopic; return *this; }
NodeBridge& NodeBridge::heartbeat(unsigned long ms) { _hbInterval = ms; return *this; }
NodeBridge& NodeBridge::debug(bool on) { _debug = on; return *this; }
NodeBridge& NodeBridge::onConnect(EventHandler cb) { _onConnect = cb; return *this; }
NodeBridge& NodeBridge::onDisconnect(EventHandler cb) { _onDisconnect = cb; return *this; }

NodeBridge& NodeBridge::on(const char* key, CommandHandler handler) {
  if (_subCount < NODEBRIDGE_MAX_SUBS) {
    _subs[_subCount].key = key;
    _subs[_subCount].handler = handler;
    _subCount++;
  } else {
    _log("too many on() handlers - increase NODEBRIDGE_MAX_SUBS");
  }
  return *this;
}

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

bool NodeBridge::begin(const char* deviceName) {
  _device = deviceName;
  if (_self != nullptr && _self != this)
    _log("WARNING: multiple NodeBridge instances - only the last begin() receives commands");
  _self   = this;

  if (_port == 0) _port = _tls ? 8883 : 1883;   // auto-pick if not set

  if (_tls) {
    if (_caCert) {
      _netSecure.setCACert(_caCert);             // validate against provided root CA
      _log("TLS enabled (server certificate validated)");
    } else {
      _netSecure.setInsecure();                  // encrypted but UNVALIDATED (quick start only)
      _log("TLS enabled - WARNING: server certificate NOT validated; pass secure(rootCA) for production");
    }
    _mqtt.setClient(_netSecure);
  } else {
    _mqtt.setClient(_net);
  }

  _mqtt.setServer(_host, _port);
  _mqtt.setCallback(_trampoline);
  _mqtt.setBufferSize(NODEBRIDGE_BUFFER_SIZE);
  _mqtt.setSocketTimeout(5);                      // bound blocking connect/read so loop() stays responsive

  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _pass);

  _log("connecting WiFi...");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) delay(200);

  if (WiFi.status() == WL_CONNECTED) {
    if (_debug) { Serial.print(F("[NodeBridge] WiFi OK, IP ")); Serial.println(WiFi.localIP()); }
    _ensureMqtt();
  } else {
    _log("WiFi not connected yet - will retry in loop()");
  }
  return connected();
}

void NodeBridge::loop() {
  if (WiFi.status() != WL_CONNECTED) {
    _ensureWifi();
  } else if (!_mqtt.connected()) {
    if (millis() - _lastReconnect > 3000) {
      _lastReconnect = millis();
      _ensureMqtt();
    }
  } else {
    _mqtt.loop();
    if (_hbInterval && millis() - _lastHb >= _hbInterval) {
      _lastHb = millis();
      char topic[160], payload[160];
      snprintf(topic, sizeof(topic), "%s/%s/heartbeat", _root, _device);
      snprintf(payload, sizeof(payload),
               "{\"rssi\":%d,\"uptime_s\":%lu,\"ip\":\"%s\"}",
               (int)WiFi.RSSI(), (unsigned long)(millis() / 1000),
               WiFi.localIP().toString().c_str());
      _mqtt.publish(topic, payload, true);
    }
  }

  // fire connect / disconnect edge events exactly once
  bool now = connected();
  if (now && !_wasConnected) { _wasConnected = true;  if (_onConnect)    _onConnect(); }
  if (!now && _wasConnected) { _wasConnected = false; if (_onDisconnect) _onDisconnect(); }
}

// ---------------------------------------------------------------------------
// connection management
// ---------------------------------------------------------------------------

bool NodeBridge::_ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  static unsigned long last = 0;
  if (millis() - last > 5000) {
    last = millis();
    _log("WiFi reconnecting...");
    WiFi.begin(_ssid, _pass);
  }
  return false;
}

bool NodeBridge::_ensureMqtt() {
  if (_mqtt.connected()) return true;
  if (WiFi.status() != WL_CONNECTED) return false;

  _log("connecting MQTT...");

  char statusTopic[160];
  snprintf(statusTopic, sizeof(statusTopic), "%s/%s/status", _root, _device);

  // Last Will: broker publishes "offline" (retained) if we drop unexpectedly.
  bool ok = _user
    ? _mqtt.connect(_device, _user, _mpass, statusTopic, 0, true, "offline")
    : _mqtt.connect(_device, nullptr, nullptr, statusTopic, 0, true, "offline");

  if (ok) {
    _log("MQTT connected");
    _mqtt.publish(statusTopic, "online", true);   // retained presence
    _resubscribe();
  } else {
    if (_debug) { Serial.print(F("[NodeBridge] MQTT failed, rc=")); Serial.println(_mqtt.state()); }
  }
  return ok;
}

void NodeBridge::_resubscribe() {
  for (uint8_t i = 0; i < _subCount; ++i) {
    char topic[160];
    _topic(topic, sizeof(topic), _subs[i].key, "set");
    _mqtt.subscribe(topic);
    if (_debug) { Serial.print(F("[NodeBridge] subscribed ")); Serial.println(topic); }
  }
}

// ---------------------------------------------------------------------------
// incoming messages
// ---------------------------------------------------------------------------

void NodeBridge::_trampoline(char* topic, uint8_t* payload, unsigned int len) {
  if (_self) _self->_handleMessage(topic, payload, len);
}

void NodeBridge::_handleMessage(char* topic, uint8_t* payload, unsigned int len) {
  for (uint8_t i = 0; i < _subCount; ++i) {
    char expect[160];
    _topic(expect, sizeof(expect), _subs[i].key, "set");
    if (strcmp(topic, expect) != 0) continue;

    char raw[NODEBRIDGE_BUFFER_SIZE];
    unsigned int n = len < sizeof(raw) - 1 ? len : sizeof(raw) - 1;
    memcpy(raw, payload, n);
    raw[n] = '\0';

    char value[NODEBRIDGE_BUFFER_SIZE];
    nbparse::extractValue(raw, value, sizeof(value));

    if (_debug) {
      Serial.print(F("[NodeBridge] cmd ")); Serial.print(_subs[i].key);
      Serial.print(F(" = "));               Serial.println(value);
    }
    if (_subs[i].handler) _subs[i].handler(Value(value));
    return;
  }
}

// ---------------------------------------------------------------------------
// publishing
// ---------------------------------------------------------------------------

void NodeBridge::_topic(char* out, size_t n, const char* key, const char* suffix) {
  if (suffix) snprintf(out, n, "%s/%s/%s/%s", _root, _device, key, suffix);
  else        snprintf(out, n, "%s/%s/%s",    _root, _device, key);
}

bool NodeBridge::_publishValue(const char* key, const char* jsonValue) {
  if (!_mqtt.connected()) { _log("send skipped (offline)"); return false; }
  char topic[160], payload[NODEBRIDGE_BUFFER_SIZE];
  _topic(topic, sizeof(topic), key);
  snprintf(payload, sizeof(payload),
           "{\"value\":%s,\"device\":\"%s\",\"ts\":%lu}",
           jsonValue, _device, (unsigned long)millis());
  return _mqtt.publish(topic, payload);
}

bool NodeBridge::send(const char* key, float value) {
  char b[32]; snprintf(b, sizeof(b), "%g", value); return _publishValue(key, b);
}
bool NodeBridge::send(const char* key, double value) {
  char b[32]; snprintf(b, sizeof(b), "%g", value); return _publishValue(key, b);
}
bool NodeBridge::send(const char* key, int value) {
  char b[16]; snprintf(b, sizeof(b), "%d", value); return _publishValue(key, b);
}
bool NodeBridge::send(const char* key, long value) {
  char b[20]; snprintf(b, sizeof(b), "%ld", value); return _publishValue(key, b);
}
bool NodeBridge::send(const char* key, bool value) {
  return _publishValue(key, value ? "true" : "false");
}
bool NodeBridge::send(const char* key, const char* value) {
  char q[NODEBRIDGE_BUFFER_SIZE]; nbparse::jsonQuote(value, q, sizeof(q)); return _publishValue(key, q);
}
bool NodeBridge::send(const char* key, const String& value) {
  return send(key, value.c_str());
}

bool NodeBridge::sendRaw(const char* topic, const char* payload, bool retained) {
  if (!_mqtt.connected()) return false;
  return _mqtt.publish(topic, payload, retained);
}

// ---------------------------------------------------------------------------
// status / logging
// ---------------------------------------------------------------------------

bool NodeBridge::connected() {
  return WiFi.status() == WL_CONNECTED && _mqtt.connected();
}

void NodeBridge::_log(const char* msg) {
  if (_debug) { Serial.print(F("[NodeBridge] ")); Serial.println(msg); }
}
