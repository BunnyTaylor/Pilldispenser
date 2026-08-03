# Architecture: Shaztech "tray" firmware → our "bin" model

**The report-back deliverable (brief §11, step 2) — now pinned to the real code.**
This maps the actual `Arduino/PillDispenser/` sketch (read in the fork) onto the
bulk-bin model, per file, with line references, and says KEEP / MODIFY / DELETE.

## The good news from reading the real code

The stock firmware is *closer to the bin model than the brief assumed*:

- **A "tray" is already one servo channel.** Every per-tray array is size 11 (index
  `1..10`, 0 unused): `trayNames`, `trayColor`, `trayHours/trayMin`, `trayEnabled`,
  `traytriggered`, `traydisptoday/traydismtoday`, … (`PillDispenser.ino:52-97`). A
  "bin" is just a tray **plus a bulk pill count plus a drop sensor.**
- **The servo stroke already exists.** `dispense()` runs a chained LVGL-timer state
  machine (`dispense_step1..5_timer`) that sweeps servo channel `traytodispense-1`
  between 180° and 70° (`Dispense_Rooutine.ino:45-103`) via `setServoPulse()`
  (`Servo_Stuff.ino:1-6`, PCA9685). **That sweep IS `singulate_one`.**
- **Repeat-stroke already exists.** `displace(tray, steps)` repeats the stroke
  `steps` times (`Dispense_Rooutine.ino:1-5, 67-80`). That is exactly our
  `dispense(bin, count)` loop — minus the sensor gate.

So the conversion is small and surgical: **add a bulk count + a sensor gate to the
existing stroke**, not rewrite the machine.

## What's genuinely missing (→ the NEW files added)

| New file (in `Arduino/PillDispenser/`) | Provides |
|----------------------------------------|----------|
| `bin_layer.h` | Shared types/prototypes for the layer (avoids Arduino auto-prototype ordering issues). |
| `Bin_Model.ino` | `pillsRemaining[11]` + low/max/dispensed-today arrays; NVS load/save in the existing `traySettings` namespace; daily-cap reset. |
| `Drop_Sensor.ino` | IR break-beam pulse counting via `attachInterrupt` → `DROP_OK / JAM / DOUBLE / UNVERIFIED`. **This is the piece that did not exist anywhere upstream.** |
| `Singulate.ino` | `singulateStroke()` / `singulateOne()` / `dispenseBinVerified()` — the atomic, sensor-gated dose loop + distinct fault alert. |

These are **additive and decoupled** — they compile alongside the stock sketch and
change no existing behavior until wired in at the 3 points in
[`../../Arduino/PillDispenser/BULK_BIN_INTEGRATION.md`](../../Arduino/PillDispenser/BULK_BIN_INTEGRATION.md).

## Per-file verdict

| File | Lines of interest | Verdict | Notes |
|------|-------------------|---------|-------|
| `PillDispenser.ino` | globals `52-97`; PCA9685/DFPlayer/NeoPixel init `132-139`; **physical dispense button** `99-103,242-282`; pin defs `25-38` | **KEEP → small add** | Add `loadPillCount(i)` in `loadSettings`/startup; expose bin globals. Real pins live here (see wiring doc): `SDA=22 SCL=27` (`33-34`), `LED=16` (`37`), servo `SERVOMIN/MAX 150/600` (`30-31`), DFPlayer `Serial2` TX=17 (`120`), button `GPIO4`. |
| `Servo_Stuff.ino` | `setServoPulse` `1-6`, `detachServo` `8-10` | **KEEP (reused as actuator)** | This *is* the servo actuator primitive `Singulate.ino` calls. |
| `Dispense_Rooutine.ino` | `dispense` `7-43`; step timers `45-103`; `displace` `1-5`; `dispensebatch` `105-130` | **MODIFY** | Keep the stroke geometry/timing. Route the batch dose through `dispenseBinVerified()` so each pill is sensor-confirmed and the count decrements. Hook point #2. |
| `Trays_Stuff.ino` | `updateTrays` `1-22`; `checkmark` `24-39`; `index_to_color/pixel` `117-147` | **MODIFY (mostly keep)** | Relabel tray→bin in UI text; show `pillsRemaining`; add low/empty/jam indicators. Color/LED helpers reused as-is. |
| `Save_Load_Stuff.ino` | `saveTraySettings` `170-204`, `loadTraySettings` `207-258`, `clearTrayPreferences` `134-167` | **MODIFY** | Call `savePillCount(i)`/`loadPillCount(i)` alongside the stock per-tray keys; add pill-count clears. Hook point #1. |
| `Check_Alerts.ino` | (missed-dose / reminder logic) | **MODIFY → EXTEND** | Keep reminders/missed-dose; add jam / double-drop / low-pill / empty as **distinct** alerts (`raiseFaultAlert` gives the fault LED+sound). |
| `LVGL_Buttons.ino` | button/event handlers | **MODIFY** | Phase A: add a "test singulate bin 0" button → `dispenseBinVerified(1,1)`. Later: bin-config save fields. |
| `Web_Stuff.ino` | web portal (870 lines) | **MODIFY** | Add per-bin fields: bulk count, pills/dose, low threshold, actuator type. Framework/routes kept. |
| `Sound_Stuff.ino` | `Playsound` `2-9` | **KEEP → +1 clip** | Reused. Add SD `3-Fault.mp3`; `raiseFaultAlert` calls `Playsound(3)`. |
| `Loading_Screen.ino`, `Wifi_Screen.ino`, `Lock_Screen.ino`, `TS_TFT_Stuff.ino` | — | **KEEP** | No mechanism dependency. |
| `libraries/ui` (SquareLine export) | tray screens, "sections" labels | **MODIFY** | Re-skin tray screens → bin screens in SquareLine, re-export. |
| **Carousel/tray *mechanism*** (30-section disc, cam+arm, springs) + PCB tray connectors | — | **DELETE (mechanical)** | Replaced by hopper + singulation disc (`bulk-bin/cad/`). In firmware the `displace/steps/fixaligment` carousel-rotation semantics (`Dispense_Rooutine.ino:52-80`) are **reinterpreted** as "singulate N", not deleted wholesale. |
| **Drop sensing** | none existed | **NEW** | `Drop_Sensor.ino`. |

### Summary
- **KEEP:** WiFi/NTP, PCA9685 servo primitive, DFPlayer audio, NeoPixel LEDs, OTA,
  web/UI framework, loading/wifi/lock screens.
- **MODIFY:** dispense routine (→ verified), persistence (+pill counts), tray UI/web
  (→ bin + counts), alerts (+fault types), SquareLine screens.
- **NEW:** `Bin_Model` / `Drop_Sensor` / `Singulate` (+`bin_layer.h`).
- **DELETE:** the physical carousel mechanism + its PCB connectors; the "30 sections
  / sections-remaining" mental model.

## Behavioral rules enforced by the NEW code (brief §9)
1. **Atomic per bin** — `dispenseBinVerified` confirms each pill via `waitForDrop`
   before `pillsRemaining[bin]--`; on jam/double it aborts + `raiseFaultAlert`.
2. **No count for unsensed pills** — decrement only on `DROP_OK`.
3. **Daily cap** — `binCapReached()` refuses a dose that would exceed `binMaxPerDay`.
4. **Persist immediately** — `savePillCount()` after every confirmed pill (survives
   power loss). *Still TODO:* a per-slot "dose fired" ledger so a reboot mid-schedule
   doesn't re-fire — see INTEGRATION hook #3.
5. **Distinct fault alert** — red flash = jam, amber = double-drop, plus `3-Fault.mp3`.

## Open decisions / not-yet-wired (honest status)
- The NEW files are **decoupled** — proven to compile, but not yet called from the
  stock dispense path. Wiring is the 3 hook points in `BULK_BIN_INTEGRATION.md`. I
  left them decoupled deliberately: rewiring the fragile LVGL-timer chain blind (no
  hardware here to test) risks silent breakage. Recommended to wire + verify on the
  bench (Phase A) before deleting the old async path.
- `binSensorPin[]` defaults to `-1` (no sensor) → `waitForDrop` returns
  `DROP_UNVERIFIED` and the stroke is trusted. **Set real pins before relying on
  verification for any safety-critical med** (docs/04-safety.md).
