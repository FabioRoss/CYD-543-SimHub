# Maintenance & Architecture

How the firmware is put together and how to change it safely.

## Design in one paragraph

The board renders the dashboard; SimHub only sends numbers. `loop()` drains the
serial port into a `Telemetry` struct (`telemetry.h`), and a 33 ms LVGL timer
pushes that struct into pre-built LVGL widgets (`dashboard_ui.h`). Nothing here
is game-specific — SimHub normalises every sim into the same properties, so the
firmware is written once and works everywhere.

## File map (single-translation-unit Arduino sketch)

| File | Responsibility |
|---|---|
| `LovelyLookalikeDash.ino` | Global state, `setup()`/`loop()`, brightness control, includes everything. |
| `lv_conf.h` | LVGL 9 config (color depth, heap, fonts). |
| `lv_bb_spi_lcd.h/.cpp` | LVGL⇆`bb_spi_lcd` display glue. **Do not modify** — from `cyd543-boilerplate`. |
| `theme.h` | Colour palette + font-size aliases (retune the look here). |
| `telemetry.h` | `Telemetry` struct, serial state machine, `LDU1` parser. |
| `settings.h` | `SavedSettings` + NVS load/save (brightness, units). |
| `touchscreen.h` | Capacitive touch → LVGL input device. |
| `dashboard_ui.h` | Widget construction (`build_dashboard`) + per-frame `update_dashboard`. |

The headers are included **once** from the `.ino` and freely reference the
globals defined there (`brightness`, `unitsMph`, `g_tel`, `g_lcd`, …). This is
the Arduino single-TU idiom, not modular C++.

## Control flow

```
setup()
  telemetryInit()            Serial.begin(115200) + defaults
  lv_init(); tick cb
  disp = lv_bb_spi_lcd_create(DISPLAY_CYD_543)
  g_lcd = driver_data->lcd   (brightness, touch access)
  loadSettings(); applyBrightness()
  touchInit()
  build_dashboard(screen); lv_screen_load(screen)
  forced first-frame render loop
  lv_timer_create(ui_refresh_timer, 33ms)   -> update_dashboard()

loop()
  telemetryPoll()            drain serial -> g_tel
  lv_timer_periodic_handler()   LVGL render + fire the 33ms UI timer
```

`update_dashboard()` also drives the rev-bar shift flash (time-based) and the
"waiting for SimHub" overlay (via `telemetryStale()`), so the UI animates even
when data pauses.

## Common changes

### Retune colours / fonts
Everything visual keys off `theme.h`. Change `COL_*` and the `FONT_*` aliases in
one place. To use a new Montserrat size, enable it in `lv_conf.h`
(`LV_FONT_MONTSERRAT_nn 1`) first.

### Move / resize widgets
All positions are absolute in `build_dashboard()` (480×272 canvas). Follow the
board rules already applied by the `mkPanel`/`mkLabel` helpers: no scroll, zero
padding, explicit background. Keep the rev bar at the top and the three tap
zones covering left/right thirds.

### Adding a new telemetry field
Append-only keeps old SimHub messages working. End-to-end:

1. **`telemetry.h`** — add a member to `struct Telemetry`; parse it in
   `telemetryParse()` using the next index, e.g. `g_tel.newThing = atoi(FLD(27));`.
   (Scale fractionals to integers — see the encoding rule in PROTOCOL.md.)
2. **`simhub/LovelyLookalike.CustomSerial.txt`** — add
   `+ format(isnull([DataCorePlugin.GameData.NewData.NewThing],0),'0') + ';'`
   **before** the final `+ '#'`, matching the new index. (Add the `;` on the
   previous last data field.)
3. **`dashboard_ui.h`** — add a widget in `build_dashboard()` and set it in
   `update_dashboard()`.
4. **`docs/PROTOCOL.md`** — add the row to the field table.

Because parsing is index-based and tolerant, a firmware that expects the new
field still works with an old message (field defaults), and vice-versa.

### Change the update rate
Set the interval in SimHub's Update Message (33 ms ≈ 30 Hz, 16 ms ≈ 60 Hz). The
firmware caps UI work at ~30 fps via the `ui_refresh_timer`; raise it there if
you send faster and want a snappier bar.

### Second page / more screens
Add another `lv_obj_t*` screen built like `build_dashboard`, and switch with
`lv_screen_load()` from a tap-zone handler (e.g. a centre zone). Persist the
active page in `SavedSettings` (bump `SETTINGS_VERSION`).

## Memory notes
- Display draw buffer = 1/10 screen in internal RAM (in `lv_bb_spi_lcd.cpp`);
  partial render mode. Enable **PSRAM** so LVGL's 128 KB heap + font glyph cache
  have room.
- `LV_MEM_SIZE` is 128 KB — do not reduce (board rule). Many Montserrat sizes
  are enabled; disable unused ones in `lv_conf.h` to reclaim flash.
- Telemetry uses fixed buffers and no `String` — no heap churn in the hot path.

## Known limitations / ideas
- It's a **look-alike**, not the real Lovely `.djson`. To show the genuine
  PC-rendered dashboard you'd stream compressed frames from a Windows companion
  app (a different, heavier architecture — intentionally not used here).
- `BrakeBias` units vary by game (percent vs 0..1). The message assumes percent;
  adjust the `*10` if your main sim differs (see the notes in the SimHub file).
- No opponent list / relative, no tyre temps yet — both are easy append-only
  additions following the steps above.

## Versioning
- `fw_version` in the `.ino` — bump per release.
- `SETTINGS_VERSION` in `settings.h` — bump whenever `SavedSettings` layout
  changes (old NVS blobs are then discarded and defaults used).
- Protocol header `LDU1` — bump to `LDU2` only for a **breaking** reorder;
  append-only changes keep `LDU1`.
