// Singulate.ino — sensor-verified singulation (the safety heart of the conversion).
//
// singulateStroke()  = one raw servo stroke (the stock 180°↔70° sweep, blocking).
// singulateOne()     = stroke + verify + count exactly one pill.
// dispenseBinVerified() = atomic per-bin dose of N pills, with jam/double-drop halt.
//
// Behavioral rules enforced (docs/02, brief §9):
//  • confirm each pill via the sensor BEFORE decrementing the stored count
//  • on jam or double-drop, abort that bin and raise a DISTINCT alert
//  • never advance the count for pills that weren't sensed
//  • refuse up-front on empty bin or daily-cap breach
#include "bin_layer.h"

// One raw stroke. Mirrors the stock dispense_step timers' angles/dwell, but runs
// synchronously so it can be gated by the sensor. Servo channel is bin-1 (as stock).
void singulateStroke(int bin) {
  int ch = bin - 1;
  setServoPulse(ch, SING_ANGLE_SCOOP);
  delay(SING_SCOOP_MS);
  setServoPulse(ch, SING_ANGLE_DROP);
  delay(SING_DROP_MS);
  detachServo(ch);                 // stop holding torque between strokes
}

SingulateResult singulateOne(int bin) {
  singulateStroke(bin);

  switch (waitForDrop(bin)) {
    case DROP_OK:
    case DROP_UNVERIFIED:          // no sensor: trust the stroke (policy in caller)
      pillsRemaining[bin]    -= 1;
      binDispensedToday[bin] += 1;
      savePillCount(bin);          // persist after each confirmed pill
      return SING_OK;

    case DROP_JAM:
      return SING_JAM;

    case DROP_DOUBLE:
    default:
      return SING_DOUBLE;
  }
}

// Dispense `n` pills from one bin, fully verified and atomic. Returns the terminal
// status; on a fault the bin is left detached and the fault alert has fired.
SingulateResult dispenseBinVerified(int bin, int n) {
  if (bin < 1 || bin > 10) return SING_EMPTY;
  if (binIsEmpty(bin))          return SING_EMPTY;
  if (binCapReached(bin, n))    return SING_CAP;   // runaway-loop guard

  for (int i = 0; i < n; i++) {
    if (binIsEmpty(bin)) return SING_EMPTY;        // ran dry mid-dose

    SingulateResult r = singulateOne(bin);
    if (r == SING_JAM) {
      raiseFaultAlert(bin, false);
      return SING_JAM;                             // halt bin; count untouched for this pill
    }
    if (r == SING_DOUBLE) {
      raiseFaultAlert(bin, true);
      return SING_DOUBLE;
    }
  }

  if (binIsLow(bin)) {
    // Proactive low-pill nudge (routine alert, not a fault). Distinct from faults.
    Playsound(1);
  }
  return SING_OK;
}

// Distinct fault signal: fast red flash + the dedicated fault MP3, clearly different
// from the routine reminder (Playsound(1)/(2)). User must add "3-Fault.mp3" to SD.
void raiseFaultAlert(int bin, bool doubleDrop) {
  for (int f = 0; f < 6; f++) {
    uint32_t c = doubleDrop ? pixels.Color(255, 80, 0)   // amber = double-drop
                            : pixels.Color(255, 0, 0);    // red   = jam
    pixels.setPixelColor(0, c);
    pixels.setPixelColor(1, c);
    pixels.show();
    delay(120);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.setPixelColor(1, pixels.Color(0, 0, 0));
    pixels.show();
    delay(120);
    esp_task_wdt_reset();
  }
  Playsound(FAULT_SOUND);
  Serial.printf("FAULT bin %d: %s\n", bin, doubleDrop ? "double-drop" : "jam");
}
