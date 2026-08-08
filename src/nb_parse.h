/*
  nb_parse.h  -  pure payload helpers for N-R_ESP32 (no Arduino deps).

  Kept header-only + dependency-free so they can be unit-tested on the host
  (see test/). NodeBridge.cpp includes this; do not add Arduino headers here.
*/

#ifndef NB_PARSE_H
#define NB_PARSE_H

#include <cstddef>
#include <cstring>

namespace nbparse {

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
inline void extractValue(const char* payload, char* out, size_t n) {
  const char* p = payload;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

  const char* start = nullptr;
  const char* end   = nullptr;

  if (*p == '{') {                                  // looks like JSON
    const char* key = strstr(p, "\"value\"");
    const char* c   = key ? strchr(key, ':') : nullptr;
    if (c) {
      c++;
      while (*c == ' ' || *c == '\t') c++;
      if (*c == '"') {                              // quoted string value
        start = ++c;
        while (*c && *c != '"') c++;
        end = c;
      } else {                                      // number / bool / null
        start = c;
        while (*c && *c != ',' && *c != '}' &&
               *c != ' ' && *c != '\r' && *c != '\n' && *c != '\t') c++;
        end = c;
      }
    } else {                                        // JSON but no "value" -> pass whole
      start = p; end = p + strlen(p);
    }
  } else if (*p == '"') {                            // raw quoted scalar
    start = ++p;
    while (*p && *p != '"') p++;
    end = p;
  } else {                                          // raw bare scalar
    start = p; end = p + strlen(p);
    while (end > start && (end[-1] == ' ' || end[-1] == '\r' ||
                           end[-1] == '\n' || end[-1] == '\t')) end--;
  }

  size_t len = (size_t)(end - start);
  if (len >= n) len = n - 1;
  memcpy(out, start, len);
  out[len] = '\0';
}

} // namespace nbparse

#endif // NB_PARSE_H
