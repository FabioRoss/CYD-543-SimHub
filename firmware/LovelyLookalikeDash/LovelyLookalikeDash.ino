/**
 * LovelyLookalikeDash — a Lovely-Dashboard-style SimHub DDU for the
 * JC4827W543C (CYD_543) ESP32-S3 board.
 *
 * The board renders the dashboard itself in LVGL and receives game-agnostic
 * telemetry from SimHub over USB serial ("Custom Serial device"). Because
 * SimHub normalises every supported title (iRacing, Le Mans Ultimate, Assetto
 * Corsa, ACC, rFactor 2, Automobilista 2, F1, ...) into the same
 * DataCorePlugin.GameData.NewData.* properties, this firmware works with all
 * of them without any per-game code.
 *
 * See docs/INSTALL.md for wiring, flashing and the SimHub setup, and
 * docs/PROTOCOL.md for the serial line format.
 *
 * Hardware / libraries: cyd543-boilerplate (capacitive touch variant).
 *   - lvgl >= 9.3
 *   - bb_spi_lcd
 *   - bb_captouch
 */

#include <lvgl.h>
#include "lv_conf.h"
#include <bb_spi_lcd.h>
#include "lv_bb_spi_lcd.h"
#include <bb_captouch.h>
#include <Preferences.h>

/* Forward declarations used inside the feature headers below. */
void applyBrightness();
void cycleBrightness();

/* Feature modules (single-translation-unit Arduino sketch: these headers
 * freely reference the globals defined just below). */
#include "telemetry.h"
#include "settings.h"
#include "touchscreen.h"
#include "dashboard_ui.h"

/* ---- global state ---- */
const char *fw_version = "1.0.0";

uint8_t  brightness = 255;    /* 0..255 */
uint8_t  unitsMph   = 0;      /* 0 km/h, 1 mph */

Preferences   preferences;
BBCapTouch    bbct;
Telemetry     g_tel;
lv_display_t *disp  = nullptr;
BB_SPI_LCD   *g_lcd = nullptr;

/* ---- brightness control ---- */
void applyBrightness() {
  if (g_lcd) g_lcd->setBrightness(brightness);
}

void cycleBrightness() {
  if      (brightness > 200) brightness = 160;
  else if (brightness > 120) brightness = 90;
  else                       brightness = 255;
  applyBrightness();
  saveSettings();
}

/* ---- periodic UI refresh (~30 fps) ---- */
static void ui_refresh_timer(lv_timer_t *t) {
  (void)t;
  update_dashboard();
}

void setup() {
  telemetryInit();              /* Serial.begin(115200) + defaults */
  delay(300);
  Serial.printf("\n[LovelyLookalikeDash] v%s booting\n", fw_version);

  lv_init();
  lv_tick_set_cb([]() { return (uint32_t)(esp_timer_get_time() / 1000ULL); });

  disp = lv_bb_spi_lcd_create(DISPLAY_CYD_543);   /* named constant — never a raw int */
  if (!disp) { Serial.println("[ERROR] Display init failed"); while (true) delay(100); }

  lv_bb_spi_lcd_t *dsc = (lv_bb_spi_lcd_t *)lv_display_get_driver_data(disp);
  g_lcd = dsc->lcd;

  loadSettings();
  applyBrightness();

  touchInit(nullptr);

  lv_obj_t *screen = lv_obj_create(NULL);
  build_dashboard(screen);
  lv_screen_load(screen);

  update_dashboard();

  /* force the first frames to the panel */
  for (int i = 0; i < 20; i++) { lv_timer_periodic_handler(); delay(5); }

  lv_timer_create(ui_refresh_timer, 33, NULL);
  Serial.println("[LovelyLookalikeDash] ready — waiting for SimHub telemetry");
}

void loop() {
  telemetryPoll();              /* drain serial, update g_tel */
  lv_timer_periodic_handler();  /* LVGL rendering + the 33 ms UI refresh timer */
}
