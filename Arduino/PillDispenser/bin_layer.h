// bin_layer.h — bulk-bin + drop-verification layer for the auto pill dispenser.
//
// This is the ADDITIVE part of the tray→bin conversion (see BULK_BIN/INTEGRATION.md).
// It sits alongside the stock firmware without changing existing behavior: it adds a
// per-bin bulk PILL COUNT and a sensor-verified singulation path. Wiring it into the
// existing dispense flow is done at the 3 hook points documented in INTEGRATION.md.
//
// The stock firmware indexes trays 1..10 (arrays are size 11, index 0 unused). We
// keep that convention exactly so a "bin" is just a tray with a bulk count + sensor.
#pragma once
#include <Arduino.h>

// ── Per-bin state (parallel to trayNames[11], trayColor[11], … in PillDispenser.ino)
extern int pillsRemaining[11];     // bulk pills currently in the bin
extern int binLowThreshold[11];    // warn at/below this many
extern int binMaxPerDay[11];       // hard daily dispense cap (runaway guard)
extern int binDispensedToday[11];  // reset at the daily reset time
extern int binSensorPin[11];       // IR receiver GPIO; -1 = no sensor wired

// Timing/geometry of one servo singulation stroke (mirrors Dispense_Rooutine.ino).
// Kept here so it's tunable in one place per the parametric-design goal.
static const int   SING_ANGLE_SCOOP  = 180; // pocket passes under the hopper
static const int   SING_ANGLE_DROP   = 70;  // pocket aligns to the drop-hole
static const int   SING_SCOOP_MS     = 300; // dwell at scoop
static const int   SING_DROP_MS      = 500; // dwell at drop (pill clears)
static const int   SENSOR_WINDOW_MS  = 800; // watch for the drop after a stroke
static const int   SENSOR_DEBOUNCE_MS= 8;   // a pill breaks the beam a few ms
static const bool  SENSOR_ACTIVE_LOW = true;// beam broken pulls the line LOW
static const int   FAULT_SOUND       = 3;   // SD "3-Fault.mp3" (user must add it)

// ── Bin helpers
bool binIsEmpty(int bin);
bool binIsLow(int bin);
bool binCapReached(int bin, int want);
void loadPillCount(int bin);   // read pillsRemaining/max/threshold from NVS
void savePillCount(int bin);   // persist them (called after each confirmed pill)
void resetDailyBinCounts();    // zero binDispensedToday[] at the daily reset

// ── Drop verification (IR break-beam pulse counting)
enum DropResult { DROP_OK, DROP_JAM, DROP_DOUBLE, DROP_UNVERIFIED };
DropResult waitForDrop(int bin);

// ── Sensor-verified singulation
enum SingulateResult { SING_OK, SING_JAM, SING_DOUBLE, SING_EMPTY, SING_CAP };
void            singulateStroke(int bin);            // one raw servo stroke, blocking
SingulateResult singulateOne(int bin);               // stroke + verify + count
SingulateResult dispenseBinVerified(int bin, int n); // atomic per-bin dose of n pills
void            raiseFaultAlert(int bin, bool doubleDrop);
