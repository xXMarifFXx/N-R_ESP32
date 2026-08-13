/*
  nb_parse.h  -  pure payload helpers for N-R_ESP32 (no Arduino deps).

  Kept header-only + dependency-free so they can be unit-tested on the host
  (see test/). NodeBridge.cpp includes this; do not add Arduino headers here.
*/

#ifndef NB_PARSE_H
#define NB_PARSE_H

#include <cstddef>
#include <cstring>
#include <cstdio>

namespace nbparse {

// A topic segment owned by NodeBridge: deliberately excludes MQTT separators
// and wildcards, JSON punctuation, spaces and control bytes.
inline bool validIdentifier(const char* s, size_t maxLen) {
  if (!s || !*s) return false;
  size_t n = 0;
  for (; s[n]; ++n) {
    const char c = s[n];
    const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-';
    if (!ok || n >= maxLen) return false;
  }
  return n <= maxLen;
}

// Match an incoming topic against <root>/<device>/<key>/set. On success returns
// true and points keyStart/keyLen at the <key> segment (within `topic`). Pure so
// it can be unit-tested on the host; used by NodeBridge::_handleMessage.
inline bool matchSetTopic(const char* topic, const char* root, const char* device,
                          const char** keyStart, size_t* keyLen) {
  char prefix[128];
  int plen = snprintf(prefix, sizeof(prefix), "%s/%s/", root, device);
  if (plen <= 0 || (size_t)plen >= sizeof(prefix)) return false;   // too long to match
  if (strncmp(topic, prefix, (size_t)plen) != 0) return false;
  const char* mid = topic + plen;                                  // "<key>/set"
  size_t midlen = strlen(mid);
  if (midlen < 4 || strcmp(mid + midlen - 4, "/set") != 0) return false;
  *keyStart = mid;
  *keyLen = midlen - 4;
  return true;
}

// Copy `in` into `out` as a JSON string literal (adds quotes, escapes " and \).
inline void jsonQuote(const char* in, char* out, size_t n) {
  size_t j = 0;
  if (n == 0) return;
  if (j < n - 1) out[j++] = '"';
  for (const char* p = in; *p && j < n - 2; ++p) {
    if (*p == '"' || *p == '\\') { out[j++] = '\\'; if (j >= n - 2) break; }
    out[j++] = *p;
  }
  if (j < n - 1) out[j++] = '"';
  out[j] = '\0';
}

// Pull the scalar command value out of an MQTT payload.
// Accepts a raw scalar ("24.5", "on", "\"hi\"") OR flat JSON {"value": <x>}.
//
// IN-PLACE variant: modifies `s` (writes a '\0' at the end of the value) and
// returns a pointer to the value within `s`. Lets callers avoid a second buffer.
inline const char* extractValueInPlace(char* s) {
  char* p = s;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

  char* start;
  char* end;

  if (*p == '{') {                                  // looks like JSON
    // Find "value" used as a KEY: the token must be followed (after optional
    // spaces) by ':'. This ignores "value" appearing as a string value elsewhere.
    char* c = nullptr;
    for (char* q = strstr(p, "\"value\""); q; q = strstr(q + 7, "\"value\"")) {
      char* a = q + 7;                              // just past  "value"
      while (*a == ' ' || *a == '\t') a++;
      if (*a == ':') { c = a; break; }
    }
    if (c) {
      c++;
      while (*c == ' ' || *c == '\t') c++;
      if (*c == '"') {                              // quoted string value
        start = c + 1; end = start;
        while (*end && *end != '"') end++;
      } else {                                      // number / bool / null
        start = c; end = c;
        while (*end && *end != ',' && *end != '}' &&
               *end != ' ' && *end != '\r' && *end != '\n' && *end != '\t') end++;
      }
    } else {                                        // JSON but no "value" -> pass whole
      start = p; end = p + strlen(p);
    }
  } else if (*p == '"') {                            // raw quoted scalar
    start = p + 1; end = start;
    while (*end && *end != '"') end++;
  } else {                                          // raw bare scalar
    start = p; end = p + strlen(p);
    while (end > start && (end[-1] == ' ' || end[-1] == '\r' ||
                           end[-1] == '\n' || end[-1] == '\t')) end--;
  }

  *end = '\0';
  return start;
}

// COPY variant (kept for host tests): copies `payload`, extracts in place, and
// left-aligns the result into `out`.
inline void extractValue(const char* payload, char* out, size_t n) {
  if (n == 0) return;
  size_t plen = strlen(payload);
  if (plen >= n) plen = n - 1;
  memcpy(out, payload, plen);
  out[plen] = '\0';
  const char* v = extractValueInPlace(out);
  if (v != out) memmove(out, v, strlen(v) + 1);
}

} // namespace nbparse

#endif // NB_PARSE_H
