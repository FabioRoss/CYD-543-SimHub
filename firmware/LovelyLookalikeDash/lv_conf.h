/**
 * @file lv_conf.h
 * LVGL 9.x configuration for the JC4827W543C (CYD_543) SimHub look-alike DDU.
 *
 * Only the options we care about are overridden here; every option NOT set
 * below falls back to LVGL's built-in default (see lv_conf_internal.h).
 *
 * Place this file where the LVGL library can find it. The simplest setup is
 * to keep it in the sketch folder and set, in lvgl/lv_conf.h or via the
 * Arduino build flag, LV_CONF_PATH — but the standard Arduino flow is to copy
 * lv_conf.h next to the `lvgl` library folder OR enable LV_CONF_INCLUDE_SIMPLE.
 * See docs/INSTALL.md for the exact steps.
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR / MEMORY
 *====================*/
#define LV_COLOR_DEPTH        16          /* RGB565 */
#define LV_COLOR_16_SWAP      0           /* the bb_spi_lcd flush_cb byte-swaps, not LVGL */

/* LVGL heap. 128 KB minimum for this board — do NOT reduce. */
#define LV_USE_STDLIB_MALLOC  LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING  LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_BUILTIN
#define LV_MEM_SIZE           (128 * 1024U)

/*====================
   HAL / TICK
 *====================*/
/* We provide the tick via lv_tick_set_cb(esp_timer_get_time()/1000) in setup(). */
#define LV_USE_OS             LV_OS_NONE

/*====================
   FEATURES USED
 *====================*/
#define LV_USE_CANVAS         1
#define LV_USE_LOG            0            /* set 1 + LV_LOG_LEVEL for LVGL debugging */

/*====================
   FONTS
 *====================*/
#define LV_FONT_MONTSERRAT_8   1
#define LV_FONT_MONTSERRAT_10  1
#define LV_FONT_MONTSERRAT_12  1
#define LV_FONT_MONTSERRAT_14  1
#define LV_FONT_MONTSERRAT_16  1
#define LV_FONT_MONTSERRAT_18  1
#define LV_FONT_MONTSERRAT_20  1
#define LV_FONT_MONTSERRAT_24  1
#define LV_FONT_MONTSERRAT_28  1
#define LV_FONT_MONTSERRAT_36  1
#define LV_FONT_MONTSERRAT_40  1
#define LV_FONT_MONTSERRAT_48  1

#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*====================
   WIDGETS (only what we use — keeps flash smaller)
 *====================*/
#define LV_USE_LABEL          1
#define LV_LABEL_TEXT_SELECTION 0
#define LV_USE_BUTTON         1
#define LV_USE_IMAGE          1
#define LV_USE_LINE           1
#define LV_USE_BAR            1
#define LV_USE_ARC            1

#endif /* LV_CONF_H */
