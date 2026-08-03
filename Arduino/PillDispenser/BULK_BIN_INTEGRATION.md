# Bulk-bin conversion — how the new files wire into the stock sketch

This sketch has been extended for the **bulk-bin auto dispenser** (loose pills in a
hopper, singulate one at a time, verified by an IR drop sensor) instead of the stock
hand-filled carousel tray. See `../../bulk-bin/` for the full project (research,
architecture map, CAD, safety).

## New files (additive — they change no stock behavior on their own)

| File | Role |
|------|------|
| `bin_layer.h` | shared types + prototypes for the layer |
| `Bin_Model.ino` | per-bin bulk count + daily cap, persisted in the `traySettings` NVS namespace |
| `Drop_Sensor.ino` | IR break-beam drop verification (`waitForDrop`) |
| `Singulate.ino` | `singulateStroke` / `singulateOne` / `dispenseBinVerified` + `raiseFaultAlert` |

They **compile alongside** the stock code and are validated (syntax-checked against
the Arduino API). They are intentionally **not yet called** from the stock dispense
path — wire them in at the 3 hook points below, then verify on the bench (Phase A)
before removing the old async carousel path.

> `#include "bin_layer.h"` at the top of `PillDispenser.ino` (with the other
> includes) so the whole sketch sees the layer's prototypes.

## Hook point #1 — persist & load the pill counts

Alongside the stock per-tray persistence in `Save_Load_Stuff.ino`:

- In **`saveTraySettings(int trayIndex)`** (`:170`), after the existing `putInt`s,
  add: `savePillCount(trayIndex);`
- In **`loadTraySettings(int trayIndex)`** (`:207`), after loading the stock keys,
  add: `loadPillCount(trayIndex);`
- In **`clearTrayPreferences(int trayIndex)`** (`:134`), also remove the new keys
  (`pillsRem%d`, `binLow%d`, `binMax%d`, `binDisp%d`) — or just call the same NVS
  `remove()` pattern.

Pill counts then survive power loss exactly like the stock tray settings.

## Hook point #2 — route a dose through the verified path

The stock batch dose is `dispensebatch()` → `dispense(tray)` /
`displace(tray, steps)` in `Dispense_Rooutine.ino`. To make a scheduled dose
**counted and verified**, dispense N pills from a bin via:

```cpp
SingulateResult r = dispenseBinVerified(tray, pillsThisDose);
// r == SING_OK        → all pills confirmed, counts decremented + persisted
// r == SING_JAM/DOUBLE → bin halted, fault alert already raised
// r == SING_EMPTY/CAP  → nothing dispensed; surface an alert
```

For **Phase A** (prove one bin before touching the scheduler), don't rewire the batch
yet — add a temporary UI/button test in `LVGL_Buttons.ino` that calls
`dispenseBinVerified(1, 1)` and watch the count decrement on a confirmed drop.

For **Phase B**, replace the body of `dispense()`/the `dispense_step*` timer chain
for a scheduled dose with `dispenseBinVerified()`. Keep the stock stroke geometry
(180°↔70°, the same angles `Singulate.ino` uses) so mechanics behave identically.

> Note on timing: `dispenseBinVerified()` is **blocking** (it `delay()`s through each
> stroke and the sensor window, pumping `lv_timer_handler()` + `esp_task_wdt_reset()`
> so the UI and watchdog stay alive). A few pills × ~1.6 s is well within the 15 s
> WDT. If you ever dose many pills at once, chunk it or convert to the timer-chain
> style — but keep the **confirm-before-decrement** ordering intact.

## Hook point #3 — daily reset + fault assets + (TODO) dose-fired ledger

- **Daily cap reset:** call `resetDailyBinCounts();` wherever the stock daily reset
  fires (the `resetHours`/`resetMin` path). Clears `binDispensedToday[]`.
- **Fault sound:** add `3-Fault.mp3` to the SD card root (next to `1-Alert.mp3`,
  `2-PillsReady.mp3`). `raiseFaultAlert()` calls `Playsound(3)` and flashes the LEDs
  (red = jam, amber = double-drop) — distinct from routine reminders.
- **Sensor pins:** set `binSensorPin[bin]` in `Bin_Model.ino` to the wired IR
  receiver GPIO for each bin. Left as `-1`, `waitForDrop` returns `DROP_UNVERIFIED`
  and the stroke is trusted (fine for bring-up; **not** for a safety-critical med).
- **TODO (not yet implemented): per-slot "dose fired" ledger.** To guarantee a reboot
  mid-schedule never re-fires or double-fires a dose, persist a per-schedule-slot
  "fired today" flag (the stock `traydisptoday%d` bool is a starting point) and check
  it before dispensing. Track this before trusting the scheduler unattended.

## Safety
Read `../../bulk-bin/docs/04-safety.md`. Drop verification is mandatory for
dangerous-if-wrong meds; split/half pills are unsupported; keep a manual backup.
