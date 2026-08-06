/*
  HiveMQCloud - connect to a HiveMQ Cloud cluster (TLS + username/password).

  HiveMQ Cloud requires an encrypted TLS connection on port 8883 plus the
  username/password you created in the HiveMQ "Access Management" tab.

  In the HiveMQ Cloud console, open your cluster and copy:
    - the cluster URL   -> looks like  xxxxxxxx.s1.eu.hivemq.cloud
    - a username / password  (Access Management -> Credentials)

  Then point your Node-RED mqtt-broker node at the SAME cluster URL / port 8883,
  tick "Use TLS", and enter the same username/password. Both ends meet in the cloud.

  Library: N-R_ESP32   (needs PubSubClient installed)
*/

#include <NodeBridge.h>

NodeBridge bridge;

void setup() {
  Serial.begin(115200);

  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("xxxxxxxx.s1.eu.hivemq.cloud")   // your cluster URL (port auto -> 8883)
        .secure()                                // TLS on. See note below to validate the cert.
        .login("YOUR_HIVEMQ_USERNAME", "YOUR_HIVEMQ_PASSWORD")
        .debug(true);

  // Example command coming from Node-RED (devices/esp32-demo/led/set)
  bridge.on("led", [](Value v) { digitalWrite(2, v.asBool()); });

  pinMode(2, OUTPUT);
  bridge.begin("esp32-demo");
}

void loop() {
  bridge.loop();

  static unsigned long last = 0;
  if (millis() - last > 2000) {
    last = millis();
    float tempC = 20.0 + (millis() % 1000) / 100.0;   // fake reading
    bridge.send("temperature", tempC);
  }
}

/*
  ---------------------------------------------------------------------------
  About .secure() and certificate validation
  ---------------------------------------------------------------------------
  .secure()  with no argument encrypts the connection but does NOT verify the
  broker's certificate. That's fine to get running, but a determined attacker
  on your network could impersonate the broker.

  To verify the server (recommended for production), pass HiveMQ Cloud's root
  CA. HiveMQ Cloud certificates are issued by Let's Encrypt (ISRG Root X1).
  Put the PEM below and use .secure(HIVEMQ_ROOT_CA):

  static const char* HIVEMQ_ROOT_CA = R"EOF(
  -----BEGIN CERTIFICATE-----
  ...ISRG Root X1 PEM here (get it from https://letsencrypt.org/certificates/)...
  -----END CERTIFICATE-----
  )EOF";

  Note: with real certificate validation, the ESP32 clock must be roughly
  correct (TLS checks cert expiry). If validation fails on boot, sync time via
  NTP first, e.g.  configTime(0, 0, "pool.ntp.org");  then bridge.begin(...).
*/
