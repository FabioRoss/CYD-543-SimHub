/**
 * @file touchscreen.h
 * Capacitive touch (GT911-class, I2C) LVGL input device for the JC4827W543C.
 * Pins per cyd543-boilerplate: SDA=8, SCL=4, INT=3, RST=-1.
 *
 * Uses the board skill's self-calibrating min/max mapping so it adapts to the
 * panel's reported range on the fly. Only coarse accuracy is needed here — the
 * only touch targets are three full-height tap zones (see dashboard_ui.h).
 */
#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <lvgl.h>
#include <bb_captouch.h>

#define TOUCH_SDA   8
#define TOUCH_SCL   4
#define TOUCH_INT   3
#define TOUCH_RST  -1

extern BBCapTouch bbct;

static TOUCHINFO ti;
static uint16_t  touchMinX = 1, touchMaxX = 480;
static uint16_t  touchMinY = 1, touchMaxY = 272;

static void touch_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (bbct.getSamples(&ti)) {
    if (ti.x[0] < touchMinX) touchMinX = ti.x[0];
    if (ti.x[0] > touchMaxX) touchMaxX = ti.x[0];
    if (ti.y[0] < touchMinY) touchMinY = ti.y[0];
    if (ti.y[0] > touchMaxY) touchMaxY = ti.y[0];
    data->point.x = map(ti.x[0], touchMinX, touchMaxX, 1,
                        lv_display_get_horizontal_resolution(NULL));
    data->point.y = map(ti.y[0], touchMinY, touchMaxY, 1,
                        lv_display_get_vertical_resolution(NULL));
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static inline void touchInit(lv_indev_t **out_indev) {
  bbct.init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touch_read);
  if (out_indev) *out_indev = indev;
}

#endif /* TOUCHSCREEN_H */
