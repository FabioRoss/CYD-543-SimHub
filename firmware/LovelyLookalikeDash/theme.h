/**
 * @file theme.h
 * Colour palette + font aliases for the SimHub look-alike DDU.
 *
 * Palette follows the CYD_543 guidance: true-black background, saturated
 * accents, greys reserved for inactive/secondary only. Tuned to echo the
 * Lovely Dashboard look (green->amber->red rev bar, cyan info accents).
 */
#ifndef THEME_H
#define THEME_H

#include <lvgl.h>

/* ---- Surfaces ---- */
#define COL_BG        lv_color_hex(0x000000)   /* true black root */
#define COL_PANEL     lv_color_hex(0x0D1117)   /* side panels / cards */
#define COL_PANEL2    lv_color_hex(0x161B22)   /* raised sub-panel */
#define COL_BORDER    lv_color_hex(0x30363D)   /* dividers */

/* ---- Text ---- */
#define COL_TEXT      lv_color_hex(0xF0F6FC)   /* primary values */
#define COL_LABEL     lv_color_hex(0x8B949E)   /* secondary labels (sparingly) */
#define COL_INACTIVE  lv_color_hex(0x30363D)   /* unlit rev segment / disabled */

/* ---- Accents ---- */
#define COL_GREEN     lv_color_hex(0x3FB950)   /* good / faster delta / low revs */
#define COL_CYAN      lv_color_hex(0x2FE6E6)   /* info accent (speed, rpm) */
#define COL_BLUE      lv_color_hex(0x3D8BFF)   /* shift flash / blue flag */
#define COL_AMBER     lv_color_hex(0xE3B341)   /* warning / mid revs */
#define COL_RED       lv_color_hex(0xF85149)   /* alert / high revs / slower delta */
#define COL_MAGENTA   lv_color_hex(0xD24DFF)   /* session-best highlight */
#define COL_WHITE     lv_color_hex(0xFFFFFF)

/* ---- Flag colours ---- */
#define COL_FLAG_YELLOW lv_color_hex(0xF2C300)
#define COL_FLAG_BLUE   lv_color_hex(0x1E6FEB)
#define COL_FLAG_WHITE  lv_color_hex(0xFFFFFF)
#define COL_FLAG_BLACK  lv_color_hex(0x555555)
#define COL_FLAG_GREEN  lv_color_hex(0x3FB950)

/* ---- Fonts (aliases so sizes can be retuned in one place) ---- */
#define FONT_GEAR     &lv_font_montserrat_48
#define FONT_SPEED    &lv_font_montserrat_40
#define FONT_XL       &lv_font_montserrat_36
#define FONT_BIG      &lv_font_montserrat_28
#define FONT_VAL      &lv_font_montserrat_24
#define FONT_VAL_SM   &lv_font_montserrat_20
#define FONT_MED      &lv_font_montserrat_16
#define FONT_LBL      &lv_font_montserrat_12
#define FONT_TINY     &lv_font_montserrat_10
#define FONT_MICRO    &lv_font_montserrat_8

#endif /* THEME_H */
