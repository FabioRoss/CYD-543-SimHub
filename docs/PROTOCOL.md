# Serial Protocol — `LDU1`

The firmware receives one telemetry **record** per update from SimHub's Custom
Serial device. This is the firmware's *own* line format (not the ESP-SimHub
"Custom Arduino" binary protocol) — deliberately simple so it can be produced by
a single SimHub NCalc expression and parsed cheaply on the ESP32.

## Framing

```
LDU1;<f1>;<f2>; ... ;<f26>#
```

- **Header** `LDU1` (field 0) — lets the parser resync mid-stream.
- Fields separated by `;`.
- **Terminator** `#` — marks end of record. No newline needed. Any `\r`/`\n`
  in the stream is ignored.
- The parser buffers bytes until `#`, then splits and parses. A record whose
  field 0 isn't `LDU1` is discarded. Buffer overflow (no `#` within
  `TEL_RX_BUFSZ` bytes) resets the buffer and resyncs.
- Records are **tolerant**: a short line just leaves later fields at defaults.

## Encoding rule (important)

All **fractional** quantities are transmitted as **scaled integers** so the wire
never contains a decimal separator. This avoids breakage on non-US Windows
locales that format `0.234` as `0,234`. The firmware rescales on receipt.

## Field table

| # | Name | SimHub source (`[...]`) | Wire encoding | Firmware field |
|---|------|--------------------------|---------------|----------------|
| 0 | header | — | literal `LDU1` | — |
| 1 | gear | `GameData.NewData.Gear` | string `R/N/1..8` | `gear` |
| 2 | speed | `GameData.NewData.SpeedKmh` | int km/h | `speedKmh` |
| 3 | rpm | `GameData.NewData.Rpms` | int | `rpm` |
| 4 | maxRpm | `GameData.NewData.MaxRpm` | int | `maxRpm` |
| 5 | rpmPct | `GameData.NewData.CarSettings_CurrentDisplayedRPMPercent` | int 0..100 (×100) | `rpmPct` |
| 6 | position | `GameData.NewData.Position` | int | `pos` |
| 7 | opponents | `GameData.NewData.OpponentsCount` | int | `oppCount` |
| 8 | currentLap | `GameData.NewData.CurrentLap` | int | `curLap` |
| 9 | totalLaps | `GameData.NewData.TotalLaps` | int (0 = timed) | `totLaps` |
| 10 | lastLap | `GameData.NewData.LastLapTime` | string `mm:ss.fff` | `lastLap` |
| 11 | bestLap | `GameData.NewData.BestLapTime` | string `mm:ss.fff` | `bestLap` |
| 12 | curLapTime | `GameData.NewData.CurrentLapTime` | string `mm:ss.fff` | `curLapT` |
| 13 | delta | `PersistantTrackerPlugin.SessionBestLiveDeltaSeconds` | int ms (×1000, signed) | `delta` (s) |
| 14 | fuel | `GameData.NewData.Fuel` | int deci-L (×10) | `fuel` (L) |
| 15 | fuel/lap | `Computed.Fuel_LitersPerLap` | int centi-L (×100) | `fuelPerLap` |
| 16 | fuelLaps | `Computed.Fuel_RemainingLaps` | int deci-laps (×10) | `fuelLaps` |
| 17 | TC | `GameData.NewData.TCLevel` | int, `-1` = n/a | `tc` |
| 18 | ABS | `GameData.NewData.ABSLevel` | int, `-1` = n/a | `absLevel` |
| 19 | brakeBias | `GameData.NewData.BrakeBias` | int deci-% (×10), `<0` = n/a | `bb` (%) |
| 20 | flags | `GameData.NewData.Flag_*` | int bitmask | `flags` |
| 21 | water | `GameData.NewData.WaterTemperature` | int °C | `water` |
| 22 | oil | `GameData.NewData.OilTemperature` | int °C | `oil` |
| 23 | pit | `GameData.NewData.PitLimiterOn` | int 0/1 | `pit` |
| 24 | drs | `DRSEnabled + DRSAvailable` | int 0/1/2 | `drs` |
| 25 | brake | `GameData.NewData.Brake` | int 0..100 | `brake` |
| 26 | throttle | `GameData.NewData.Throttle` | int 0..100 | `throttle` |

### Flag bitmask (field 20)
| Bit | Value | Flag |
|---|---|---|
| 0 | 1 | Yellow |
| 1 | 2 | Blue |
| 2 | 4 | White |
| 3 | 8 | Black |
| 4 | 16 | Checkered |
| 5 | 32 | Green |

Display priority (highest first): Checkered → Black → Blue → Yellow → White → Green.

### DRS (field 24)
`0` none/not available · `1` available (armed) · `2` open. Derived as
`DRSEnabled + DRSAvailable`.

### Laptime sentinel
SimHub reports unavailable laptimes as `TimeSpan.Zero` → `00:00.000`. The
firmware maps `00:00.000` (and empty) to `--:--.---`.

## Why these properties are game-agnostic

Everything above the plugin prefix `DataCorePlugin.GameData.NewData.*` is
SimHub's **normalised** game data: SimHub's per-game data readers populate the
same property names regardless of the sim. `Computed.*` are SimHub-computed
(fuel math), and `PersistantTrackerPlugin.*` is SimHub's lap/delta tracker.
None of them are game-specific, so one message drives every supported title.

Fields a given game doesn't provide come through as `0`/`-1` (via `isnull`) and
the UI hides or dashes them.

## Extending the protocol
See [MAINTENANCE.md](MAINTENANCE.md#adding-a-new-telemetry-field) for the
end-to-end steps (append a field to keep it backward-compatible).
