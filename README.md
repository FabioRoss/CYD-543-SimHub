# CYD-543-SimHub — LovelyLookalikeDash

A **SimHub racing DDU** for the **JC4827W543C** ("CYD_543") ESP32-S3 board — a
480×272 LVGL dashboard styled after the popular **Lovely Dashboard**, driven by
live telemetry from SimHub over a plain USB cable.

It works with **every major title** SimHub supports — iRacing, Le Mans Ultimate,
Assetto Corsa, Assetto Corsa Competizione, rFactor 2, Automobilista 2, F1, and
more — because it consumes SimHub's *game-agnostic* telemetry properties. There
is **no per-game code** and **no Windows driver** to install.

> **What this is (and isn't).** This is a native firmware **look-alike**: the
> board renders the dashboard itself in LVGL from telemetry values. It is *not*
> the actual Lovely Dashboard `.djson` (that is a PC-rendered Dash Studio DDU
> that only SimHub can render). This approach is crisp, fast (60 fps), low
> latency, needs no companion PC app, and matches the CYD's native 480×272.

Screen layout (480×272):

```
┌───────────────────────────────────────────────────────────┐
│ ▁▁▁▁▁▁▁▁ rev / shift-light bar (20 segments) ▁▁▁▁▁▁▁▁▁▁▁▁ │
│ ┌─────────┐  ▐          ▌  ┌─────────┐                     │
│ │ LAST    │  ▐          ▌  │ RPM     │                     │
│ │ BEST    │  ▐   GEAR   ▌  │ FUEL    │                     │
│ │ POS     │  ▐  SPEED   ▌  │ FUELLAP │                     │
│ │ LAP     │ brk  km/h thr │ TC  ABS  │                     │
│ └─────────┘               └─────────┘                     │
│ ┌────┐ PIT     DELTA          BB 54%      H2O 92° OIL110°  │
│ │FLAG│ DRS    -0.234                                       │
│ └────┘                                                     │
└───────────────────────────────────────────────────────────┘
```

## How it works

```
   ┌────────────┐   USB serial (Custom Serial device, 115200)   ┌──────────────┐
   │  SimHub PC │  ── "LDU1;N;212;7450;...;#"  telemetry line ─▶ │  CYD_543     │
   │  + any sim │                                                │  ESP32-S3    │
   └────────────┘                                                │  LVGL DDU    │
        ▲  reads iRacing / LMU / AC / ACC / rF2 / AMS2 / F1...   └──────────────┘
        └─ SimHub normalises them all into DataCorePlugin.GameData.NewData.*
```

1. SimHub's **Custom Serial device** plugin sends one delimited line (~30–60 Hz)
   built from a single NCalc expression (`simhub/LovelyLookalike.CustomSerial.txt`).
2. The firmware parses the line and updates the LVGL widgets.
3. The board shows up as a standard **USB COM port** on Windows 11 (built-in
   `usbser.sys` / CH340) — nothing to install.

## Dashboard elements

- 20-segment **rev / shift-light bar** (green → amber → red, blue flash at the
  shift point), game-normalised.
- Huge **gear** and **speed** (km/h or mph, toggle by touch), flanked by
  **brake/throttle** pedal bars.
- **Last / Best** lap, **position**, **lap count**.
- **RPM**, **fuel** remaining, **fuel-laps** left.
- **Delta** to session best (green = faster, red = slower).
- **Flags**, **pit limiter**, **DRS**, **TC/ABS**, **brake bias**, water/oil temps.
- **"Waiting for SimHub"** overlay when telemetry stops.

### Touch controls
- Tap the **left third** → toggle km/h ↔ mph.
- Tap the **right third** → cycle brightness (100 % → 63 % → 35 %).
- Both settings persist across reboots (NVS).

## Repository layout

```
CYD-543-SimHub/
├── README.md
├── firmware/LovelyLookalikeDash/     ← Arduino sketch (open the .ino)
│   ├── LovelyLookalikeDash.ino       ← setup/loop, globals
│   ├── lv_conf.h                     ← LVGL config
│   ├── lv_bb_spi_lcd.h/.cpp          ← display glue (from cyd543-boilerplate)
│   ├── theme.h                       ← colours + font aliases
│   ├── telemetry.h                   ← serial protocol parser
│   ├── settings.h                    ← brightness/units persistence
│   ├── touchscreen.h                 ← capacitive touch input
│   └── dashboard_ui.h                ← LVGL layout + per-frame update
├── simhub/LovelyLookalike.CustomSerial.txt   ← the SimHub update message
└── docs/
    ├── INSTALL.md                    ← flashing + SimHub setup, step by step
    ├── PROTOCOL.md                   ← wire format + field table
    └── MAINTENANCE.md                ← architecture + how to extend
```

## Quick start

1. **Flash** the sketch — see [docs/INSTALL.md](docs/INSTALL.md) for board
   settings and required libraries.
2. In SimHub, enable **Custom Serial devices**, pick the board's COM port at
   **115200**, and paste the expression from
   [`simhub/LovelyLookalike.CustomSerial.txt`](simhub/LovelyLookalike.CustomSerial.txt)
   as an Update Message at 33 ms.
3. Launch any supported game — the dashboard comes alive.

## License

MIT — see headers. The Lovely Dashboard name belongs to its authors; this
project is an independent, telemetry-driven tribute and ships none of their
assets.
