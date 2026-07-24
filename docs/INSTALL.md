# Installation & Setup

Two parts: **(A)** flash the firmware to the board, **(B)** configure SimHub to
stream telemetry to it. No Windows driver is required beyond the standard USB
serial COM port.

---

## A. Flash the firmware (Arduino IDE)

### 1. Boards manager
Install the **esp32 by Espressif Systems** core (v3.x) via
*Boards Manager*. Select board **"ESP32S3 Dev Module"**.

### 2. Board settings (Tools menu)
| Setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | **Enabled** *(if your board uses the ESP32-S3 native USB port)* |
| Flash Size | 8MB (or as per your board) |
| PSRAM | **OPI PSRAM** (enable — LVGL/display buffers benefit) |
| Partition Scheme | any with ≥1.9MB app (e.g. "8M with spiffs") |
| Upload Speed | 921600 |

> **USB CDC note.** Many JC4827W543C boards expose the ESP32-S3 **native USB**
> (set *USB CDC On Boot = Enabled*, and `Serial` is that USB port). Some board
> revisions use a **CH340** UART bridge instead — then *USB CDC On Boot* is
> irrelevant and `Serial` is UART0 over the CH340. Either way the firmware talks
> to SimHub over `Serial` at 115200; just pick whichever COM port appears.

### 3. Libraries (Library Manager)
| Library | Version |
|---|---|
| **lvgl** | ≥ 9.3.0 |
| **bb_spi_lcd** | latest (must include `DISPLAY_CYD_543`) |
| **bb_captouch** | latest |

Do **not** install TFT_eSPI / Adafruit_GFX — they conflict with `bb_spi_lcd`.

### 4. Make LVGL find `lv_conf.h`
LVGL reads its config from a file named `lv_conf.h` that must sit **next to the
`lvgl` library folder**, not only inside the sketch. Do this once:

1. Copy `firmware/LovelyLookalikeDash/lv_conf.h` to your Arduino libraries
   folder so it sits **beside** (not inside) the `lvgl` folder, e.g.:
   ```
   Arduino/libraries/lv_conf.h      <-- copy here
   Arduino/libraries/lvgl/
   ```
2. Keep the copy in the sketch folder too (the sketch `#include`s it). Both
   copies must be identical.

*(Alternative: add the build flag `-DLV_CONF_INCLUDE_SIMPLE` and let the sketch
folder copy be picked up. The copy-beside-lvgl method above is the most
reliable in the stock Arduino IDE.)*

### 5. Open & upload
Open `firmware/LovelyLookalikeDash/LovelyLookalikeDash.ino` (all the `.h`/`.cpp`
files load with it as tabs). Compile and upload. On boot the Serial Monitor
(115200) prints:
```
[LovelyLookalikeDash] v1.0.0 booting
[LovelyLookalikeDash] ready — waiting for SimHub telemetry
```
and the screen shows **"WAITING FOR SIMHUB…"**.

> Close the Arduino Serial Monitor before starting SimHub — only one program can
> own the COM port at a time.

---

## B. Configure SimHub

1. Install/enable the **Custom Serial devices** plugin
   (SimHub → *Add device* → *Custom serial device*, or *Settings → Plugins*).
2. Add a device, choose the board's **COM port**, set **baud = 115200**.
3. Open the **Update Messages** tab, add **one** message:
   - Paste the whole expression from
     `simhub/LovelyLookalike.CustomSerial.txt`.
   - Set the send interval to **33 ms** (~30 Hz). Use 16 ms for ~60 Hz.
   - Leave any "append newline / CR-LF" option **off** (the `#` terminates the
     record).
4. Save. SimHub now streams telemetry; the dashboard updates the moment a
   supported game is running (and even shows menu/idle values before that).

### Verifying
- No game running: values are mostly 0 / dashes — that's expected.
- In a session: gear, speed, revs, laps, delta all move.
- If the screen stays on "WAITING FOR SIMHUB…", the line isn't arriving — see
  Troubleshooting.

---

## Touch controls
| Gesture | Action |
|---|---|
| Tap **left third** of the screen | toggle **km/h ↔ mph** |
| Tap **right third** of the screen | cycle **brightness** (100→63→35 %) |

Both are saved to NVS and restored on next boot.

---

## Troubleshooting

| Symptom | Fix |
|---|---|
| `Assembler messages: unknown opcode or format name 'typedef'` (on `stdint.h` / `_default_types.h`) | LVGL's `.S` assembly files include `lv_conf.h` during the **assembler** pass, so `lv_conf.h` must not `#include` any C header unguarded. This project's `lv_conf.h` is already assembler-safe (no unguarded `#include`). **Make sure the copy LVGL actually reads — the one next to the `lvgl` library folder (`Documents/Arduino/libraries/lv_conf.h`) — is this exact file.** Re-copy it after any update. |
| Blank / white screen | Ensure `lv_bb_spi_lcd_create(DISPLAY_CYD_543)` used the **named constant** and `bb_spi_lcd` is new enough to define it. Confirm PSRAM enabled. |
| "WAITING FOR SIMHUB…" never clears | Wrong COM port or baud; Arduino Serial Monitor still open; or the Update Message wasn't added. Confirm 115200 and the `#`-terminated line in SimHub's "log" view. |
| Garbled / partial values | Baud mismatch, or a newline option is on in SimHub. Keep 115200 and `#` terminator only. |
| Numbers with commas break fields | The protocol already sends scaled **integers** to avoid locale decimal commas — make sure you used the provided expression unmodified. |
| Rev bar never fills | That game doesn't provide `CarSettings_CurrentDisplayedRPMPercent`; the firmware auto-falls back to Rpms/MaxRpm — verify `MaxRpm` is non-zero for the car. |
| Touch axes swapped/off | Touch auto-calibrates its range as you tap; the three big zones are forgiving. For precise mapping see comments in `touchscreen.h`. |
| Speed unit wrong | Tap the left third to toggle km/h ↔ mph. |
