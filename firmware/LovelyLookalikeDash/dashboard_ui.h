/**
 * @file dashboard_ui.h
 * Lovely-Dashboard-style DDU layout for 480x272, built with LVGL 9 and
 * absolute positioning (per cyd543-boilerplate rules).
 *
 * Layout
 *   [ 20-segment rev / shift-light bar .............................. ] top
 *   [ LEFT panel ][ brake | GEAR / SPEED | throttle ][ RIGHT panel ]  middle
 *   [ flag | pit/drs |   DELTA   | bb / temps ...................... ] bottom
 *
 * update_dashboard() is called whenever a fresh telemetry record arrives (and
 * on a slow timer) — it reads the global g_tel and pushes values into the
 * widgets. All strings are game-agnostic; nothing here knows which sim is
 * running because SimHub already normalised the data.
 */
#ifndef DASHBOARD_UI_H
#define DASHBOARD_UI_H

#include <lvgl.h>
#include <math.h>
#include "theme.h"
#include "telemetry.h"

/* Provided by the .ino */
extern uint8_t brightness;
extern uint8_t unitsMph;
extern void    cycleBrightness();
extern void    saveSettings();

/* ---- Rev bar geometry ---- */
#define REV_SEGMENTS 20
#define SEG_W        21
#define SEG_H        16
#define SEG_GAP      2
#define SEG_Y        3

struct DashUI {
  lv_obj_t *leds[REV_SEGMENTS];
  /* left panel */
  lv_obj_t *lastVal, *bestVal, *posVal, *lapVal;
  /* centre */
  lv_obj_t *gear, *speed, *speedUnit, *brakeBar, *throttleBar;
  /* right panel */
  lv_obj_t *rpmVal, *fuelVal, *fuelLapsVal, *aidsVal;
  /* bottom */
  lv_obj_t *flagBox, *flagLbl, *pitLbl, *drsLbl, *deltaVal, *bbVal, *tempVal;
  /* overlay */
  lv_obj_t *waitBox, *waitLbl;
};

static DashUI ui;

/* ------------------------------------------------------------------ */
/*  small builders                                                    */
/* ------------------------------------------------------------------ */
static lv_obj_t *mkLabel(lv_obj_t *p, int x, int y, int w,
                         const lv_font_t *f, lv_color_t c,
                         lv_text_align_t a, const char *txt) {
  lv_obj_t *l = lv_label_create(p);
  if (w > 0) { lv_obj_set_width(l, w); lv_obj_set_style_text_align(l, a, 0); }
  lv_obj_set_style_text_font(l, f, 0);
  lv_obj_set_style_text_color(l, c, 0);
  lv_label_set_text(l, txt);
  lv_obj_set_pos(l, x, y);
  return l;
}

static lv_obj_t *mkPanel(lv_obj_t *p, int x, int y, int w, int h,
                         lv_color_t bg, int radius) {
  lv_obj_t *o = lv_obj_create(p);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(o, 0, 0);
  lv_obj_set_style_border_width(o, 0, 0);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_set_style_bg_color(o, bg, 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  return o;
}

static lv_obj_t *mkPedalBar(lv_obj_t *p, int x, int y, int w, int h,
                            lv_color_t indic) {
  lv_obj_t *b = lv_bar_create(p);
  lv_obj_set_pos(b, x, y);
  lv_obj_set_size(b, w, h);
  lv_bar_set_range(b, 0, 100);
  lv_bar_set_value(b, 0, LV_ANIM_OFF);
  lv_obj_set_style_radius(b, 2, 0);
  lv_obj_set_style_bg_color(b, COL_PANEL2, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(b, 2, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(b, indic, LV_PART_INDICATOR);
  return b;
}

/* ------------------------------------------------------------------ */
/*  tap-zone handlers                                                 */
/* ------------------------------------------------------------------ */
static void zoneUnits_cb(lv_event_t *e) {
  (void)e;
  unitsMph = unitsMph ? 0 : 1;
  saveSettings();
}
static void zoneBright_cb(lv_event_t *e) {
  (void)e;
  cycleBrightness();
}

/* ------------------------------------------------------------------ */
/*  build                                                             */
/* ------------------------------------------------------------------ */
static void build_dashboard(lv_obj_t *scr) {
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(scr, COL_BG, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(scr, 0, 0);

  /* --- rev / shift bar --- */
  int totalW = REV_SEGMENTS * SEG_W + (REV_SEGMENTS - 1) * SEG_GAP;
  int startX = (480 - totalW) / 2;
  for (int i = 0; i < REV_SEGMENTS; i++) {
    lv_obj_t *s = lv_obj_create(scr);
    lv_obj_set_pos(s, startX + i * (SEG_W + SEG_GAP), SEG_Y);
    lv_obj_set_size(s, SEG_W, SEG_H);
    lv_obj_remove_flag(s, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(s, 0, 0);
    lv_obj_set_style_border_width(s, 0, 0);
    lv_obj_set_style_radius(s, 2, 0);
    lv_obj_set_style_bg_color(s, COL_INACTIVE, 0);
    lv_obj_set_style_bg_opa(s, LV_OPA_COVER, 0);
    ui.leds[i] = s;
  }

  /* --- left panel --- */
  mkPanel(scr, 4, 26, 150, 182, COL_PANEL, 6);
  mkLabel(scr, 14, 32,   0, FONT_TINY,   COL_LABEL, LV_TEXT_ALIGN_LEFT, "LAST");
  ui.lastVal = mkLabel(scr, 14, 44, 132, FONT_VAL_SM, COL_TEXT, LV_TEXT_ALIGN_LEFT, "--:--.---");
  mkLabel(scr, 14, 76,   0, FONT_TINY,   COL_LABEL, LV_TEXT_ALIGN_LEFT, "BEST");
  ui.bestVal = mkLabel(scr, 14, 88, 132, FONT_VAL_SM, COL_MAGENTA, LV_TEXT_ALIGN_LEFT, "--:--.---");
  mkLabel(scr, 14, 122,  0, FONT_TINY,   COL_LABEL, LV_TEXT_ALIGN_LEFT, "POS");
  ui.posVal  = mkLabel(scr, 14, 132, 132, FONT_BIG, COL_TEXT, LV_TEXT_ALIGN_LEFT, "--");
  mkLabel(scr, 14, 172,  0, FONT_TINY,   COL_LABEL, LV_TEXT_ALIGN_LEFT, "LAP");
  ui.lapVal  = mkLabel(scr, 60, 172, 90, FONT_MED, COL_TEXT, LV_TEXT_ALIGN_RIGHT, "-");

  /* --- centre --- */
  ui.brakeBar    = mkPedalBar(scr, 158, 30, 10, 176, COL_RED);
  ui.throttleBar = mkPedalBar(scr, 312, 30, 10, 176, COL_GREEN);
  ui.gear      = mkLabel(scr, 170, 28, 140, FONT_GEAR,  COL_TEXT, LV_TEXT_ALIGN_CENTER, "N");
  ui.speed     = mkLabel(scr, 170, 120, 140, FONT_SPEED, COL_CYAN, LV_TEXT_ALIGN_CENTER, "0");
  ui.speedUnit = mkLabel(scr, 170, 178, 140, FONT_LBL,   COL_LABEL, LV_TEXT_ALIGN_CENTER, "KM/H");

  /* --- right panel --- */
  mkPanel(scr, 326, 26, 150, 182, COL_PANEL, 6);
  mkLabel(scr, 336, 32,   0, FONT_TINY,   COL_LABEL, LV_TEXT_ALIGN_LEFT, "RPM");
  ui.rpmVal      = mkLabel(scr, 336, 44, 130, FONT_BIG, COL_TEXT, LV_TEXT_ALIGN_LEFT, "0");
  mkLabel(scr, 336, 84,   0, FONT_TINY,   COL_LABEL, LV_TEXT_ALIGN_LEFT, "FUEL");
  ui.fuelVal     = mkLabel(scr, 336, 96, 130, FONT_VAL_SM, COL_TEXT, LV_TEXT_ALIGN_LEFT, "-- L");
  mkLabel(scr, 336, 128,  0, FONT_TINY,   COL_LABEL, LV_TEXT_ALIGN_LEFT, "FUEL LAPS");
  ui.fuelLapsVal = mkLabel(scr, 336, 140, 130, FONT_VAL_SM, COL_AMBER, LV_TEXT_ALIGN_LEFT, "--");
  ui.aidsVal     = mkLabel(scr, 336, 178, 130, FONT_LBL, COL_LABEL, LV_TEXT_ALIGN_LEFT, "TC -  ABS -");

  /* --- bottom bar --- */
  mkPanel(scr, 4, 212, 472, 56, COL_PANEL, 6);
  ui.flagBox = mkPanel(scr, 10, 220, 92, 40, COL_PANEL2, 4);
  ui.flagLbl = mkLabel(ui.flagBox, 0, 0, 92, FONT_MED, COL_WHITE, LV_TEXT_ALIGN_CENTER, "");
  lv_obj_align(ui.flagLbl, LV_ALIGN_CENTER, 0, 0);
  ui.pitLbl  = mkLabel(scr, 110, 220, 44, FONT_LBL, COL_RED,  LV_TEXT_ALIGN_LEFT, "");
  ui.drsLbl  = mkLabel(scr, 110, 244, 44, FONT_LBL, COL_GREEN, LV_TEXT_ALIGN_LEFT, "");

  mkLabel(scr, 168, 216, 144, FONT_TINY, COL_LABEL, LV_TEXT_ALIGN_CENTER, "DELTA");
  ui.deltaVal = mkLabel(scr, 168, 228, 144, FONT_XL, COL_LABEL, LV_TEXT_ALIGN_CENTER, "--.---");

  ui.bbVal   = mkLabel(scr, 326, 220, 146, FONT_LBL, COL_TEXT,  LV_TEXT_ALIGN_RIGHT, "BB --");
  ui.tempVal = mkLabel(scr, 326, 244, 146, FONT_LBL, COL_LABEL, LV_TEXT_ALIGN_RIGHT, "H2O --  OIL --");

  /* --- tap zones (invisible, on top) --- */
  lv_obj_t *zL = lv_obj_create(scr);       /* left third -> toggle units */
  lv_obj_set_pos(zL, 0, 26); lv_obj_set_size(zL, 160, 182);
  lv_obj_set_style_bg_opa(zL, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(zL, 0, 0);
  lv_obj_remove_flag(zL, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(zL, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(zL, zoneUnits_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *zR = lv_obj_create(scr);       /* right third -> cycle brightness */
  lv_obj_set_pos(zR, 320, 26); lv_obj_set_size(zR, 160, 182);
  lv_obj_set_style_bg_opa(zR, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(zR, 0, 0);
  lv_obj_remove_flag(zR, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(zR, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(zR, zoneBright_cb, LV_EVENT_CLICKED, NULL);

  /* --- "waiting for SimHub" overlay (hidden until stale) --- */
  ui.waitBox = mkPanel(scr, 0, 0, 480, 272, COL_BG, 0);
  lv_obj_set_style_bg_opa(ui.waitBox, LV_OPA_70, 0);
  ui.waitLbl = mkLabel(ui.waitBox, 0, 0, 480, FONT_BIG, COL_LABEL,
                       LV_TEXT_ALIGN_CENTER, "WAITING FOR SIMHUB...");
  lv_obj_align(ui.waitLbl, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(ui.waitBox, LV_OBJ_FLAG_HIDDEN);
}

/* ------------------------------------------------------------------ */
/*  update                                                            */
/* ------------------------------------------------------------------ */
static void update_rev_bar(int rpmPct) {
  bool shift = rpmPct >= 99;
  bool flashOn = (millis() / 80) % 2;   /* ~6 Hz blink at the shift point */
  for (int i = 0; i < REV_SEGMENTS; i++) {
    bool lit = rpmPct >= (i + 1) * (100 / REV_SEGMENTS);
    lv_color_t c;
    if (!lit) {
      c = COL_INACTIVE;
    } else if (shift) {
      c = flashOn ? COL_BLUE : COL_INACTIVE;
    } else if (i < 12) {
      c = COL_GREEN;
    } else if (i < 17) {
      c = COL_AMBER;
    } else {
      c = COL_RED;
    }
    lv_obj_set_style_bg_color(ui.leds[i], c, 0);
  }
}

static void update_dashboard() {
  char buf[40];

  /* stale-link overlay */
  if (telemetryStale())
    lv_obj_remove_flag(ui.waitBox, LV_OBJ_FLAG_HIDDEN);
  else
    lv_obj_add_flag(ui.waitBox, LV_OBJ_FLAG_HIDDEN);

  /* Rev-bar fill: prefer SimHub's game-normalised percent; fall back to
   * rpm/maxRpm for the rare game that doesn't populate it. */
  int pct = g_tel.rpmPct;
  if (pct <= 0 && g_tel.maxRpm > 0)
    pct = constrain((int)((long)g_tel.rpm * 100 / g_tel.maxRpm), 0, 100);
  update_rev_bar(pct);

  /* gear */
  lv_label_set_text(ui.gear, g_tel.gear[0] ? g_tel.gear : "N");

  /* speed + unit */
  int spd = unitsMph ? (int)lroundf(g_tel.speedKmh * 0.621371f) : g_tel.speedKmh;
  snprintf(buf, sizeof(buf), "%d", spd);
  lv_label_set_text(ui.speed, buf);
  lv_label_set_text(ui.speedUnit, unitsMph ? "MPH" : "KM/H");

  /* pedals */
  lv_bar_set_value(ui.brakeBar, g_tel.brake, LV_ANIM_OFF);
  lv_bar_set_value(ui.throttleBar, g_tel.throttle, LV_ANIM_OFF);

  /* rpm */
  snprintf(buf, sizeof(buf), "%d", g_tel.rpm);
  lv_label_set_text(ui.rpmVal, buf);

  /* laps / pos */
  lv_label_set_text(ui.lastVal, g_tel.lastLap);
  lv_label_set_text(ui.bestVal, g_tel.bestLap);
  if (g_tel.pos > 0) snprintf(buf, sizeof(buf), "P%d", g_tel.pos);
  else               snprintf(buf, sizeof(buf), "--");
  lv_label_set_text(ui.posVal, buf);
  if (g_tel.totLaps > 0) snprintf(buf, sizeof(buf), "%d/%d", g_tel.curLap, g_tel.totLaps);
  else                   snprintf(buf, sizeof(buf), "%d", g_tel.curLap);
  lv_label_set_text(ui.lapVal, buf);

  /* fuel */
  snprintf(buf, sizeof(buf), "%.1f L", g_tel.fuel);
  lv_label_set_text(ui.fuelVal, buf);
  snprintf(buf, sizeof(buf), "%.1f", g_tel.fuelLaps);
  lv_label_set_text(ui.fuelLapsVal, buf);

  /* driver aids */
  {
    char tcs[8], abss[8];
    if (g_tel.tc >= 0)       snprintf(tcs, sizeof(tcs), "%d", g_tel.tc);
    else                     strcpy(tcs, "-");
    if (g_tel.absLevel >= 0) snprintf(abss, sizeof(abss), "%d", g_tel.absLevel);
    else                     strcpy(abss, "-");
    snprintf(buf, sizeof(buf), "TC %s  ABS %s", tcs, abss);
    lv_label_set_text(ui.aidsVal, buf);
  }

  /* delta */
  if (!g_tel.everReceived) {
    lv_label_set_text(ui.deltaVal, "--.---");
    lv_obj_set_style_text_color(ui.deltaVal, COL_LABEL, 0);
  } else {
    snprintf(buf, sizeof(buf), "%+.3f", g_tel.delta);
    lv_label_set_text(ui.deltaVal, buf);
    lv_obj_set_style_text_color(ui.deltaVal,
                                g_tel.delta <= 0.0f ? COL_GREEN : COL_RED, 0);
  }

  /* brake bias */
  if (g_tel.bb >= 0) snprintf(buf, sizeof(buf), "BB %.0f%%", g_tel.bb);
  else               strcpy(buf, "BB --");
  lv_label_set_text(ui.bbVal, buf);

  /* temps (no degree glyph — not in the built-in Montserrat range) */
  snprintf(buf, sizeof(buf), "H2O %dC  OIL %dC", g_tel.water, g_tel.oil);
  lv_label_set_text(ui.tempVal, buf);

  /* flags (priority: checkered > black > blue > yellow > white > green) */
  const char *fTxt = ""; lv_color_t fBg = COL_PANEL2; lv_color_t fFg = COL_WHITE;
  if (g_tel.flags & FLAG_CHECKERED)   { fTxt = "FINISH"; fBg = COL_PANEL2; fFg = COL_WHITE; }
  else if (g_tel.flags & FLAG_BLACK)  { fTxt = "BLACK";  fBg = COL_FLAG_BLACK; fFg = COL_WHITE; }
  else if (g_tel.flags & FLAG_BLUE)   { fTxt = "BLUE";   fBg = COL_FLAG_BLUE;  fFg = COL_WHITE; }
  else if (g_tel.flags & FLAG_YELLOW) { fTxt = "YELLOW"; fBg = COL_FLAG_YELLOW; fFg = COL_BG; }
  else if (g_tel.flags & FLAG_WHITE)  { fTxt = "WHITE";  fBg = COL_FLAG_WHITE; fFg = COL_BG; }
  else if (g_tel.flags & FLAG_GREEN)  { fTxt = "GREEN";  fBg = COL_FLAG_GREEN; fFg = COL_BG; }
  lv_obj_set_style_bg_color(ui.flagBox, fBg, 0);
  lv_label_set_text(ui.flagLbl, fTxt);
  lv_obj_set_style_text_color(ui.flagLbl, fFg, 0);

  /* pit / drs */
  lv_label_set_text(ui.pitLbl, g_tel.pit ? "PIT" : "");
  if (g_tel.drs == 2)      { lv_label_set_text(ui.drsLbl, "DRS"); lv_obj_set_style_text_color(ui.drsLbl, COL_GREEN, 0); }
  else if (g_tel.drs == 1) { lv_label_set_text(ui.drsLbl, "DRS"); lv_obj_set_style_text_color(ui.drsLbl, COL_AMBER, 0); }
  else                     { lv_label_set_text(ui.drsLbl, ""); }
}

#endif /* DASHBOARD_UI_H */
