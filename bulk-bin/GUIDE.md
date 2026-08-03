# Build Guide — Bulk-Bin Auto Pill Dispenser

A follow-along sheet: what to buy, what to print, and the exact order to do things.
Assumes you have a 3D printer and can order from Amazon, but **no** electronics/
soldering tools yet. Target: 4–6 medications ("bins").

> **Read `docs/04-safety.md` first.** This is a DIY device that dispenses medicine.
> Keep a manual backup pillbox; drop-verification is mandatory for
> dangerous-if-wrong meds; split/half pills are not supported.

**Work in phases — don't buy everything at once.** Phase 1–2 (≈$120 of parts +
tools) proves the electronics and one bin. Only then buy the rest and print the
mechanicals. This is the single most important tip: **prove one bin before scaling.**

- **Phase 1** — Get the board running (flash the stock firmware, confirm screen/UI).
- **Phase 2** — One bin on a breadboard: servo + sensor + our verified-dispense code.
- **Phase 3** — Print & tune ONE singulation module to your real pill.
- **Phase 4** — Replicate to N bins over the shared chute; build the frame.
- **Phase 5** — (optional) custom PCB to replace the breadboard.

Rough budget: **electronics ≈ $80–140**, **one-time tools ≈ $120–200**, **filament ≈ $25**.

---

## 1. Tools to buy (you said you gave your kits away)

You need these before Phase 2. ⭐ = don't skip.

| Tool | ~Price | Why / what to search |
|------|-------:|----------------------|
| ⭐ Digital calipers | $20 | You **measure your pills** with these to parametrize the disc. Essential. |
| ⭐ Multimeter | $25–35 | Check 5V/GND, continuity, find wiring faults. Essential for debugging. |
| ⭐ Temperature-controlled soldering iron kit | $25–45 | Many kits include stand, tips, solder, pump. Search "soldering iron kit 60W adjustable". |
| Solder | $8–12 | Rosin-core 60/40 (leaded, easiest) or lead-free. 0.8 mm. |
| Wire strippers | $12 | Self-adjusting is nice. |
| Flush cutters | $8 | Trim leads/zip ties. |
| Needle-nose pliers | $8 | |
| Helping-hands / PCB holder w/ magnifier | $15–25 | Holds parts while soldering. |
| Heat-shrink assortment + mini heat gun (or a lighter) | $12 | Insulate solder joints. |
| Precision screwdriver set | $12 | Servo horns, small screws. |
| Hot-glue gun + sticks | $12 | Mount servos/sensors to printed parts. |
| Electrical tape, zip ties, small tweezers | $10 | Misc. |

If money's tight, prioritize the four ⭐ items first (calipers, multimeter, iron, solder).

---

## 2. Electronics to buy

### Phase 1–2 (buy first — proves the board + one bin)

| Qty | Part | ~Price | Notes / search terms |
|----:|------|-------:|----------------------|
| 1 | **ESP32-2432S028 "CYD"** 2.8" resistive-touch board | $12–18 | The exact board this firmware targets. Search "ESP32-2432S028 CYD 2.8 resistive". Get the **resistive** 2.8" version. |
| 1 | USB cable for the CYD | $5 | Most CYDs flash over **micro-USB** on the board itself; check your unit's port and match it. |
| 1 | **PCA9685** 16-channel PWM servo driver breakout (I²C) | $7–12 | Buy a 2-pack. Search "PCA9685 16 channel servo driver". |
| 3 | **SG90** 9 g micro servos (buy a 10-pack) | $18–25 | One per bin + spares (tuning eats a couple). |
| 1 | **IR break-beam sensor pair** (or a 5-pack) | $6/pair | Emitter + receiver across the drop chute. Search "IR break beam sensor 5mm pair". *Alt:* "slotted photo-interrupter LM393 module" if your chute is narrow. Get one to start. |
| 1 | **5 V power supply, ≥3 A** (4–5 A better) | $9–13 | Powers the servos. Search "5V 4A power supply barrel". |
| 1 | Barrel-jack-to-screw-terminal adapter (pack) | $6 | Connects the supply to the breadboard rail. |
| 1 | **1000 µF** electrolytic cap (16 V+), + a few 0.1 µF ceramics | $7 | Bulk cap across the servo 5 V rail (absorbs stall spikes). |
| 1 | Breadboard (full-size) + a half-size | $8 | |
| 1 | Jumper-wire kit (M-M, M-F, F-F) | $7 | |
| 1 | Resistor assortment kit | $8 | For 4.7 kΩ I²C pull-ups if your PCA9685 lacks them. |

### Phase 3–4 (buy once one bin works)

| Qty | Part | ~Price | Notes |
|----:|------|-------:|-------|
| N-1 | More SG90 servos + IR sensor pairs | — | Enough for your total bin count (already have 1 each). |
| 1 | **DFPlayer Mini** + **microSD** (≤8 GB) + **8 Ω ~2 W speaker** (~30 mm) | $15 | Audio alarms (from base project). |
| 1 | **WS2812B** LEDs — a short strip (you need 2) | $8 | Status light. Search "WS2812B strip". |
| 1 | **74AHCT125** logic level shifter | $1–6 | Cleans up the 3.3→5 V LED data line (base uses it). Optional on short wiring. |
| — | Servo extension wires / Dupont crimps | $8 | Reach from bins to the board. |

### Optional / situational
- **28BYJ-48 stepper + ULN2003 driver** (5-pack, ~$13) — per-bin upgrade if a pill won't index reliably with a servo.
- **MCP23017 I²C GPIO expander** (~$6) — only if you run out of ESP32 pins for the 4–6 sensors (they share the I²C bus).
- Rubber feet, a small project enclosure, or reuse the base project's printed case.

> **Money-saver:** everything except the CYD is generic and cheap in multipacks.
> Buy the CYD, one PCA9685, a servo 10-pack, and one IR sensor to do Phases 1–2 for ~$60.

---

## 3. What to 3D-print

Source files: `bulk-bin/cad/` (parametric OpenSCAD). Ready-to-slice STLs for the
**default 9×4 mm round tablet** are in `bulk-bin/cad/stl/`. **Re-tune to your pill
first** (see Phase 3) — don't print the defaults blind.

Install **OpenSCAD** (free). Edit `params.scad`, then run `cad/render.sh` (or
File → Export STL in the GUI).

| Part | Qty | Print settings |
|------|----:|----------------|
| `singulation_disc.stl` | 1 per bin (+ a few while tuning) | 0.12–0.16 mm layers, **~100 % infill** (thin part, crisp pockets), no supports. |
| `disc_housing.stl` | 1 per bin | 15–20 % infill; light supports at the drop-hole taper. |
| `hopper.stl` (+ lid — uncomment in `hopper.scad`) | 1 per bin | 15 % infill; no supports. |
| `manifold_chute.stl` | 1 | 15 % infill; supports under the funnels if steep. **Set `bin_count` in `manifold_chute.scad` to your build**, then re-render. |
| Frame / enclosure | 1 | Reuse the base project's printed case + LCD/speaker mounts where possible (see the repo's `Guides/` PDFs); design a simple frame later in Phase 4. |

Filament: **PLA** is fine indoors (easy); **PETG** if it'll sit anywhere warm.
~1 spool total.

> **Food-contact note:** 3D-printed parts are **not food-grade** — layer lines trap
> residue. Contact with pills is brief, but keep parts clean/dry, wash before first
> use, and don't reuse a bin for a different med without cleaning. This is DIY, not a
> pharmacy device.

---

## 4. Software setup

Everything is in your fork; the tricky display config is **already done** (the fork
bundles a pre-configured `TFT_eSPI/User_Setup.h` and `lv_conf.h`).

### 4a. Prove the board works FIRST (no coding)
The base project ships **precompiled `.bin` files** (in its GitHub Releases). Flashing
those first confirms your CYD is healthy before you touch code.
1. Install Python + `esptool` (`pip install esptool`), or use the "ESP32 Flash
   Download Tool".
2. Download the release `.bin`s (bootloader + partitions + firmware).
3. Flash per the base **README / `Guides/`** (`esptool.py --chip esp32 write_flash …`).
4. Power it — you should get the stock touchscreen UI. **If the screen works, your
   board and toolchain path are good.** (This stock firmware is the old carousel
   version — that's fine, it's just a smoke test.)

### 4b. Build from source (needed for our bin code)
1. Install **Arduino IDE 2.x**.
2. **ESP32 board support:** File → Preferences → Additional Boards Manager URLs →
   add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   → Boards Manager → install **esp32 by Espressif**.
3. **Libraries:** the fork bundles them in `Arduino/libraries/`. Easiest: set your
   Arduino **sketchbook location** (Preferences) to the fork's `Arduino/` folder, so
   it uses the bundled, pre-configured `TFT_eSPI`, `lvgl`, `ui`, `lv_conf.h`, etc.
   (Don't install your own TFT_eSPI/LVGL — version/config mismatches are the #1 cause
   of a blank screen.)
4. **Open** `Arduino/PillDispenser/PillDispenser.ino`.
5. **Board settings:** Tools → Board → **ESP32 Dev Module**; Partition Scheme →
   **Minimal SPIFFS** (per the note atop the sketch); default other settings.
6. **Upload the web assets (SPIFFS):** the `data/` folder (web portal HTML) needs the
   filesystem uploaded once — use the ESP32 "Sketch Data Upload" / LittleFS-SPIFFS
   uploader plugin, or accept that the web portal is inactive until you do.
7. **Compile & upload.** First compile is slow. If the screen comes up and matches the
   stock UI, you're ready to add our code.

### 4c. Wire in the bulk-bin layer
Our new files are already in the sketch folder and compile alongside the stock code
(`bin_layer.h`, `Bin_Model.ino`, `Drop_Sensor.ino`, `Singulate.ino`,
`Dose_Ledger.ino`). They do nothing until you hook them in — follow
**`Arduino/PillDispenser/BULK_BIN_INTEGRATION.md`**. For Phase 2 you only need the
one small test button in step 5b below.

---

## 5. Phase 2 — one bin on a breadboard (the important milestone)

Goal: press a button → servo does one singulation stroke → the IR sensor confirms a
dropped bead → the on-screen/serial count decrements. Prove this before any mechanics.

### 5a. Wire it (full pin map in `docs/03-breadboard-wiring.md`)
- **Power:** 5 V supply → PCA9685 **V+** and a breadboard rail; **1000 µF cap** across
  that rail. **Common ground** between the supply, PCA9685, servo, sensor, and ESP32.
  **Do NOT power servos from the ESP32.**
- **I²C:** ESP32 **GPIO 22 = SDA**, **GPIO 27 = SCL** → PCA9685 SDA/SCL; PCA9685 VCC
  (logic) → 3V3. Add 4.7 kΩ pull-ups on SDA/SCL if needed.
- **Servo (bin 1):** signal → PCA9685 **channel 0**; servo V+/GND → 5 V rail / common GND.
- **IR sensor (bin 1):** receiver OUT → a free ESP32 input GPIO; emitter/receiver
  power per the module (3V3 or 5 V) / common GND. Point the beam across where pills fall.

### 5b. Tiny test hook (Phase-2 only)
In `Bin_Model.ino`, set the sensor pin you used, e.g.:
```cpp
int binSensorPin[11] = {-1, /*bin1*/ 35, -1,-1,-1,-1,-1,-1,-1,-1,-1};
```
Give bin 1 some stock pills to count and a schedule name via the normal UI, then add a
temporary test action. The simplest is to reuse the physical dispense button in
`PillDispenser.ino`'s `loop()` — call:
```cpp
dispenseBinVerified(1, 1);   // singulate 1 pill from bin 1, verified
```
Drop a bead through the beam when the servo strokes. Watch the Serial monitor:
- confirmed drop → count decrements, `SING_OK`.
- no drop (jam) → LEDs flash **red**, `3-Fault.mp3` if present, count unchanged.
- two beads → LEDs flash **amber** (double-drop), bin halts.

> Add **`3-Fault.mp3`** to the microSD root (alongside `1-Alert.mp3`, `2-PillsReady.mp3`)
> so faults sound distinct from reminders.

### 5c. Pass criteria
Servo strokes cleanly, the sensor reliably registers exactly one bead as one pulse,
and jam/double-drop behave as above. **Now you can trust the electronics.**

---

## 6. Phase 3 — print & tune ONE singulation module

1. **Measure your pill** with the calipers: round or oblong? diameter, thickness (and
   length/width if oblong).
2. Edit `bulk-bin/cad/params.scad`: set `pill_shape` and the four `PILL` numbers.
3. `./render.sh` (or export in OpenSCAD) → new `singulation_disc.stl` + housing.
4. Print, assemble on the servo, load loose pills in the hopper.
5. Tune `pocket_side_clear`, `pocket_chamfer`, `wiper_gap` until it reliably takes
   **exactly one** pill per stroke. Re-render/reprint the disc as needed.
6. **Reliability gate:** ≥ **200 consecutive correct single-drops** before you trust
   that bin (per the build brief). Different pills → different discs.

---

## 7. Phase 4 — scale to N bins + assemble

- Print `disc_housing` + `hopper` + `singulation_disc` ×N (each tuned to its pill).
- Set `bin_count` in `manifold_chute.scad`, re-render, print the chute.
- Wire servos to PCA9685 channels 0…N-1 (no extra ESP32 pins needed) and one IR
  sensor per bin (use an MCP23017 if you run short of input pins).
- Wire in the scheduler + persistence + alerts hooks (INTEGRATION #1 and #3), and
  route scheduled doses through `dispenseBinVerified()` (INTEGRATION #2).
- Mount everything on a frame so all drop-holes feed the shared chute → cup.

## 8. Phase 5 (optional) — custom PCB
Adapt the base project's PCB or design fresh in KiCad to replace the breadboard.
Only worthwhile once the mechanics are proven. See build brief §7.

---

## Safety recap (full version in `docs/04-safety.md`)
- Not child-proof — add a locking lid if any risk; keep away from kids/pets.
- Drop-verification **required** for narrow-therapeutic-index meds (anticoagulants,
  cardiac, insulin) — consider a proven commercial device for those.
- **No split/half pills.** Keep a **manual backup** pillbox and a paper med list.
- Printed parts aren't food-grade; keep clean. This automates timing/counting of an
  already-correct prescription — it doesn't replace your pharmacist.

## If you get stuck
Tell me which phase and what happened (screen blank? servo jitter? sensor not
counting? compile error?) and I'll walk you through it. The most common snags:
blank screen = wrong TFT_eSPI/LVGL (use the bundled ones); servo brown-out = servos on
the ESP32's 5 V instead of their own rail; sensor miscounts = add/adjust debounce or
reposition the beam.
