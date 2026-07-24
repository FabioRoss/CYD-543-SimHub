/**
 * @file telemetry.h
 * Serial protocol parser for the SimHub "Custom Serial" update message.
 *
 * WIRE FORMAT (one record, terminated by '#'):
 *
 *   LDU1;gear;speedKmh;rpm;maxRpm;rpmPct;pos;oppCount;curLap;totLaps;
 *   lastLap;bestLap;curLapT;delta;fuel;fuelPerLap;fuelLaps;tc;abs;bb;
 *   flags;water;oil;pit;drs;brake;throttle#
 *
 * - Fields are ';' separated, the record ends with '#'. No newline needed.
 * - Index 0 is the literal header "LDU1" used to resync mid-stream.
 * - Times (lastLap/bestLap/curLapT) arrive PRE-FORMATTED as "mm:ss.fff".
 * - Missing/short records are tolerated: absent trailing fields keep defaults.
 *
 * The exact SimHub NCalc formula that produces this line lives in
 * ../../simhub/LovelyLookalike.CustomSerial.txt and docs/PROTOCOL.md.
 */
#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>

#define TEL_RX_BAUD     115200
#define TEL_RX_BUFSZ    320
#define TEL_MAX_FIELDS  40
#define TEL_STALE_MS    1500   /* no record for this long => "waiting for SimHub" */

/* Flag bitmask (must match the formula in the SimHub message). */
#define FLAG_YELLOW    0x01
#define FLAG_BLUE      0x02
#define FLAG_WHITE     0x04
#define FLAG_BLACK     0x08
#define FLAG_CHECKERED 0x10
#define FLAG_GREEN     0x20

struct Telemetry {
  char  gear[4];        /* "R" "N" "1".."8" */
  int   speedKmh;
  int   rpm;
  int   maxRpm;
  int   rpmPct;         /* 0..100, game-normalised rev-bar fill */
  int   pos;
  int   oppCount;
  int   curLap;
  int   totLaps;        /* 0 => timed race */
  char  lastLap[12];    /* "mm:ss.fff" */
  char  bestLap[12];
  char  curLapT[12];
  float delta;          /* seconds vs session best, signed (- = faster) */
  float fuel;           /* litres remaining */
  float fuelPerLap;     /* litres/lap */
  float fuelLaps;       /* laps of fuel left */
  int   tc;             /* -1 => not available in this game */
  int   absLevel;       /* -1 => not available */
  float bb;             /* brake bias % front, -1 => n/a */
  int   flags;          /* bitmask above */
  int   water;          /* deg C */
  int   oil;            /* deg C */
  int   pit;            /* 0/1 pit limiter */
  int   drs;            /* 0 none/na, 1 available, 2 open */
  int   brake;          /* 0..100 */
  int   throttle;       /* 0..100 */

  uint32_t lastRxMs;    /* millis() of last valid record */
  bool  everReceived;   /* true once we have parsed at least one record */
};

extern Telemetry g_tel;

static char     s_rx[TEL_RX_BUFSZ];
static uint16_t s_rxLen = 0;

static inline void telemetryInit() {
  Serial.begin(TEL_RX_BAUD);
  memset(&g_tel, 0, sizeof(g_tel));
  strcpy(g_tel.gear, "N");
  strcpy(g_tel.lastLap, "--:--.---");
  strcpy(g_tel.bestLap, "--:--.---");
  strcpy(g_tel.curLapT, "--:--.---");
  g_tel.tc = -1; g_tel.absLevel = -1; g_tel.bb = -1;
}

static inline bool telemetryStale() {
  return (!g_tel.everReceived) ||
         (millis() - g_tel.lastRxMs > TEL_STALE_MS);
}

/* Copy a laptime field, mapping the "no lap yet" sentinel to dashes. */
static inline void copyLap(char *dst, const char *src, size_t n) {
  if (src == nullptr || src[0] == 0 || strcmp(src, "00:00.000") == 0) {
    strncpy(dst, "--:--.---", n);
  } else {
    strncpy(dst, src, n);
  }
  dst[n - 1] = 0;
}

/* Parse one complete record (without the trailing '#') into g_tel. */
static inline void telemetryParse(char *rec) {
  /* Split in place on ';' */
  char *f[TEL_MAX_FIELDS];
  int nf = 0;
  char *p = rec;
  f[nf++] = p;
  while (*p && nf < TEL_MAX_FIELDS) {
    if (*p == ';') { *p = 0; f[nf++] = p + 1; }
    p++;
  }
  if (nf < 1 || strcmp(f[0], "LDU1") != 0) return;   /* not our record */

  /* Helper: field value or "" if absent. */
  #define FLD(i) ((i) < nf ? f[i] : (char *)"")

  strncpy(g_tel.gear, FLD(1)[0] ? FLD(1) : "N", sizeof(g_tel.gear));
  g_tel.gear[sizeof(g_tel.gear) - 1] = 0;
  g_tel.speedKmh   = atoi(FLD(2));
  g_tel.rpm        = atoi(FLD(3));
  g_tel.maxRpm     = atoi(FLD(4));
  g_tel.rpmPct     = constrain(atoi(FLD(5)), 0, 100);
  g_tel.pos        = atoi(FLD(6));
  g_tel.oppCount   = atoi(FLD(7));
  g_tel.curLap     = atoi(FLD(8));
  g_tel.totLaps    = atoi(FLD(9));
  copyLap(g_tel.lastLap, FLD(10), sizeof(g_tel.lastLap));
  copyLap(g_tel.bestLap, FLD(11), sizeof(g_tel.bestLap));
  copyLap(g_tel.curLapT, FLD(12), sizeof(g_tel.curLapT));
  /* Fractionals are sent as scaled integers (locale-proof: no decimal
   * separator on the wire). See docs/PROTOCOL.md. */
  g_tel.delta      = atoi(FLD(13)) / 1000.0f;   /* milliseconds -> s */
  g_tel.fuel       = atoi(FLD(14)) / 10.0f;     /* deci-litres -> L */
  g_tel.fuelPerLap = atoi(FLD(15)) / 100.0f;    /* centi-litres -> L/lap */
  g_tel.fuelLaps   = atoi(FLD(16)) / 10.0f;     /* deci-laps -> laps */
  g_tel.tc         = FLD(17)[0] ? atoi(FLD(17)) : -1;
  g_tel.absLevel   = FLD(18)[0] ? atoi(FLD(18)) : -1;
  { int bbDeci = FLD(19)[0] ? atoi(FLD(19)) : -1;   /* deci-percent */
    g_tel.bb = (bbDeci < 0) ? -1.0f : bbDeci / 10.0f; }
  g_tel.flags      = atoi(FLD(20));
  g_tel.water      = atoi(FLD(21));
  g_tel.oil        = atoi(FLD(22));
  g_tel.pit        = atoi(FLD(23));
  g_tel.drs        = atoi(FLD(24));
  g_tel.brake      = constrain(atoi(FLD(25)), 0, 100);
  g_tel.throttle   = constrain(atoi(FLD(26)), 0, 100);
  #undef FLD

  g_tel.lastRxMs     = millis();
  g_tel.everReceived = true;
}

/*
 * Drain the serial port. Returns true if at least one complete record was
 * parsed this call (so the caller can refresh the UI). Non-blocking.
 */
static inline bool telemetryPoll() {
  bool got = false;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '#') {                 /* record terminator */
      s_rx[s_rxLen] = 0;
      telemetryParse(s_rx);
      s_rxLen = 0;
      got = true;
    } else if (c == '\r' || c == '\n') {
      /* tolerate stray line endings */
    } else {
      if (s_rxLen < TEL_RX_BUFSZ - 1) {
        s_rx[s_rxLen++] = c;
      } else {
        s_rxLen = 0;               /* overflow: drop and resync */
      }
    }
  }
  return got;
}

#endif /* TELEMETRY_H */
