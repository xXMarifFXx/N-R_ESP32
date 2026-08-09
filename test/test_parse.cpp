/*
  Host-side golden tests for the N-R_ESP32 payload parser.
  These include the REAL library helpers (src/nb_parse.h) — no Arduino needed.

  Run:   g++ -std=c++11 -Wall -Wextra -I../src test_parse.cpp -o test_parse && ./test_parse
  (or from the repo root:  g++ -std=c++11 -Isrc test/test_parse.cpp -o /tmp/t && /tmp/t)
*/

#include <cstdio>
#include <cstring>
#include "nb_parse.h"

static int fails = 0;

static void checkExtract(const char* in, const char* want) {
  char out[128];
  nbparse::extractValue(in, out, sizeof(out));
  bool ok = !strcmp(out, want);
  printf("  %-34s -> \"%s\"%s\n", in, out, ok ? "" : "   << MISMATCH");
  if (!ok) { printf("        expected \"%s\"\n", want); fails++; }
}

static void checkQuote(const char* in, const char* want) {
  char out[128];
  nbparse::jsonQuote(in, out, sizeof(out));
  bool ok = !strcmp(out, want);
  printf("  jsonQuote(%-16s) -> %s%s\n", in, out, ok ? "" : "   << MISMATCH");
  if (!ok) { printf("        expected %s\n", want); fails++; }
}

int main() {
  printf("extractValue:\n");
  checkExtract("on", "on");
  checkExtract("off", "off");
  checkExtract("24.5", "24.5");
  checkExtract("  1  ", "1");
  checkExtract("\"hello\"", "hello");
  checkExtract("{\"value\":24.5}", "24.5");
  checkExtract("{\"value\":\"on\"}", "on");
  checkExtract("{\"value\":true}", "true");
  checkExtract("{ \"value\" : 28 }", "28");
  checkExtract("{\"value\":\"on\",\"device\":\"x\"}", "on");
  checkExtract("{\"other\":1}", "{\"other\":1}");            // no value key -> whole payload
  checkExtract("{\"other\":\"value\"}", "{\"other\":\"value\"}"); // "value" as a value, not a key
  checkExtract("{\"note\":\"x\",\"value\":5}", "5");         // value key after another field
  checkExtract("{\"myvalue\":9}", "{\"myvalue\":9}");        // "myvalue" must not match "value"
  checkExtract("{ \"value\" :7}", "7");                      // spaces before colon still match

  printf("matchSetTopic:\n");
  { const char* k; size_t kl;
    bool ok = nbparse::matchSetTopic("devices/esp32-demo/led/set", "devices", "esp32-demo", &k, &kl);
    bool good = ok && kl == 3 && !strncmp(k, "led", 3);
    printf("  led/set -> %s (len %zu)%s\n", ok ? "match" : "no", ok ? kl : 0, good ? "" : "   << MISMATCH");
    if (!good) fails++;
  }
  { const char* k; size_t kl;
    if (nbparse::matchSetTopic("devices/other/led/set", "devices", "esp32-demo", &k, &kl)) { printf("  wrong device matched << MISMATCH\n"); fails++; }
    else printf("  wrong device rejected\n");
    if (nbparse::matchSetTopic("devices/esp32-demo/temperature", "devices", "esp32-demo", &k, &kl)) { printf("  missing /set matched << MISMATCH\n"); fails++; }
    else printf("  telemetry (no /set) rejected\n");
  }

  printf("jsonQuote:\n");
  checkQuote("hi", "\"hi\"");
  checkQuote("a\"b", "\"a\\\"b\"");
  checkQuote("a\\b", "\"a\\\\b\"");

  printf(fails ? "\nFAILED %d\n" : "\nALL PASSED\n", fails);
  return fails ? 1 : 0;
}
