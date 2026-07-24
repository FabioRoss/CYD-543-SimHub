/**
 * @file settings.h
 * Persisted user settings (brightness, speed units) in NVS via Preferences.
 * Bump SETTINGS_VERSION whenever the SavedSettings layout changes — stale
 * blobs are then discarded and safe defaults are used.
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

#define SETTINGS_VERSION 1

/* Globals owned by the sketch (declared in the .ino). */
extern uint8_t brightness;   /* 0..255 backlight */
extern uint8_t unitsMph;     /* 0 = km/h, 1 = mph */

extern Preferences preferences;

struct SavedSettings {
  uint8_t version;           /* ALWAYS first */
  uint8_t brightness;
  uint8_t unitsMph;
};

static inline void saveSettings() {
  SavedSettings s{};
  s.version    = SETTINGS_VERSION;
  s.brightness = brightness;
  s.unitsMph   = unitsMph;
  preferences.begin("lddu", false);
  size_t w = preferences.putBytes("cfg", &s, sizeof(s));
  preferences.end();
  Serial.printf("[Settings] saved %u bytes (bri=%u mph=%u)\n",
                (unsigned)w, brightness, unitsMph);
}

static inline void loadSettings() {
  preferences.begin("lddu", true);
  size_t len = preferences.getBytesLength("cfg");
  SavedSettings s{};
  if (len == sizeof(SavedSettings)) preferences.getBytes("cfg", &s, sizeof(s));
  preferences.end();
  if (len != sizeof(SavedSettings) || s.version != SETTINGS_VERSION) {
    Serial.println("[Settings] no valid blob, using defaults");
    return;                  /* defaults already set in the .ino */
  }
  brightness = s.brightness ? s.brightness : 255;
  unitsMph   = s.unitsMph ? 1 : 0;
  Serial.printf("[Settings] loaded (bri=%u mph=%u)\n", brightness, unitsMph);
}

#endif /* SETTINGS_H */
