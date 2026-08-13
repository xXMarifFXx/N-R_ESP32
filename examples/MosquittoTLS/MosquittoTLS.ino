/*
  MosquittoTLS - connect to YOUR VPS Mosquitto broker with FULL certificate
  validation (Let's Encrypt / ISRG Root X1) + username/password.

  This is the production-grade version of the HiveMQCloud example: instead of
  secure() (which does NOT verify the server), it passes the ISRG Root X1 root
  CA so the ESP32 confirms it is really talking to your broker — no MITM.

  Prereqs on the VPS: Mosquitto with a Let's Encrypt cert on port 8883
  (see mosquitto-setup.sh). Certificate validation needs the ESP32 clock to be
  correct, so we sync time over NTP before connecting.

  Library: N-R_ESP32   (needs PubSubClient installed)
*/

#include <WiFi.h>
#include <time.h>
#include <NodeBridge.h>
#include <NodeBridgeCerts.h>

const char* MQTT_USERNAME = "esp32user";

NodeBridge bridge;

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  // 1) WiFi up first (needed for NTP)
  WiFi.mode(WIFI_STA);
  WiFi.begin("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println(" connected");

  // 2) Sync the clock — TLS validation rejects the cert if time is wrong.
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time");
  time_t now = time(nullptr);
  while (now < 1700000000) { delay(300); Serial.print("."); now = time(nullptr); }
  Serial.println(" ok");

  // 3) Connect to your VPS Mosquitto with the CA -> certificate is VALIDATED.
  bridge.wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD")
        .broker("mqtt.example.com")              // your VPS domain (port auto -> 8883)
        .secure(NODEBRIDGE_ISRG_ROOT_X1)         // validate against Let's Encrypt root
        .login(MQTT_USERNAME, "YOUR_BROKER_PASSWORD")
        .debug(true);

  bridge.on("led", [](Value v) { digitalWrite(2, v.asBool()); });

  // For username-scoped brokers such as mqtt.mariffb.my, begin() MUST use the
  // MQTT username. That account is authorized only for devices/<username>/#.
  bridge.begin(MQTT_USERNAME);
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
