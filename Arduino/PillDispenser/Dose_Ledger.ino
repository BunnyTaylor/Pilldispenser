// Dose_Ledger.ino — "dose fired" ledger so a reboot never re-fires or double-fires
// a scheduled dose (brief §9 rule; docs/02 behavioral rule #4 / INTEGRATION hook #3).
//
// The stock scheduler fires each bin once per day at trayHours[bin]:trayMin[bin].
// We record the day-of-year (tm_yday) on which a bin's scheduled dose was dispensed.
// Before firing a scheduled dose, the scheduler checks doseFiredToday(bin, yday);
// after a successful dose it calls markDoseFiredToday(bin, yday). Persisted to the
// same traySettings NVS namespace so it survives a power cut; a new day (different
// yday) naturally clears it.
//
// Additive: nothing calls these until the scheduler is wired (INTEGRATION hook #3).
#include "bin_layer.h"

int doseFiredYday[11] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

void loadDoseLedger() {
  char key[24];
  preferences.begin("traySettings", true);
  for (int bin = 1; bin <= 10; bin++) {
    snprintf(key, sizeof(key), "firedYday%d", bin);
    doseFiredYday[bin] = preferences.getInt(key, -1);
  }
  preferences.end();
}

bool doseFiredToday(int bin, int yday) {
  if (bin < 1 || bin > 10) return false;
  return doseFiredYday[bin] == yday;
}

void markDoseFiredToday(int bin, int yday) {
  if (bin < 1 || bin > 10) return;
  doseFiredYday[bin] = yday;
  char key[24];
  preferences.begin("traySettings", false);
  snprintf(key, sizeof(key), "firedYday%d", bin);
  preferences.putInt(key, yday);
  preferences.end();
}
