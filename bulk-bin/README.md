# Bulk-Bin Auto Pill Dispenser

This folder is the home of the **bulk-bin conversion** of this dispenser: instead of
the stock hand-filled 30-section carousel tray, each medication lives **loose in a
bulk hopper** and is **singulated** — dropped one pill at a time — on a schedule into
a shared cup, with **every drop verified by an IR sensor**. Targets 4–6 medications.

It's built on top of `Shaztech/Pilldispenser` (this repo is a fork): we reuse the
entire ESP32-CYD + PCA9685 + DFPlayer + WS2812B + WiFi/NTP/web/Telegram/OTA stack and
replace only the mechanism + add drop verification.

## Where things are

```
bulk-bin/
├── docs/
│   ├── 00-build-brief.md                     original build brief
│   ├── 01-research-synthesis.md              open-source + commercial/patent pull (incl. MedaCube)
│   ├── 02-architecture-keep-modify-delete.md the tray→bin map, pinned to real files/lines
│   ├── 03-breadboard-wiring.md               BOM, power rules, REAL pin map
│   └── 04-safety.md                          safety requirements + failure modes
└── cad/                                      parametric OpenSCAD: disc, housing, hopper, chute

Arduino/PillDispenser/                        (the firmware — Arduino needs code in the sketch folder)
├── bin_layer.h  Bin_Model.ino  Drop_Sensor.ino  Singulate.ino   ← NEW bulk-bin layer
└── BULK_BIN_INTEGRATION.md                   how the new layer wires into the stock sketch
```

## Status (honest)

- **Docs + architecture map:** done, and now pinned to the real firmware
  (filenames + line numbers) after reading the fork.
- **Firmware layer:** the NEW bulk-bin code (`Bin_Model` / `Drop_Sensor` /
  `Singulate` + `bin_layer.h`) is written and **syntax-checked against the Arduino
  API**. It is **additive and decoupled** — it compiles alongside the stock sketch
  and changes nothing until wired in at the 3 hook points in
  `Arduino/PillDispenser/BULK_BIN_INTEGRATION.md`.
- **Not done (needs hardware / your call):** wiring the layer into the stock dispense
  path and proving it on the bench (Phase A); the per-slot "dose-fired" ledger; the
  SquareLine UI re-skin (tray→bin); CAD rendered/tuned to real pills; PCB (Phase E).
- **Full firmware compile** was not run here — the ESP32 toolchain + CYD-specific
  `TFT_eSPI`/`lv_conf` setup is the on-machine path from the base project's Guides.
  The new files were validated separately against Arduino API stubs.

## Next steps

1. **Bench Phase A:** wire one SG90 (PCA9685 ch 0) + one IR break-beam per
   `docs/03`, set `binSensorPin[1]`, add a test button → `dispenseBinVerified(1,1)`,
   confirm a dropped bead decrements the count and a jam raises the fault alert.
2. **Print + tune** one singulation module from `cad/` to your primary pill; hit the
   ≥200-correct-drops reliability gate before trusting it.
3. Then Phases B→E per `docs/00-build-brief.md` §3.

See the top-level project safety notes before building or relying on this device:
`docs/04-safety.md`.
