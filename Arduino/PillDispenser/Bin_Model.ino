// Bin_Model.ino — bulk pill-count state for each bin (tray→bin conversion).
//
// Adds a per-bin bulk count + daily cap on top of the stock tray arrays, persisted
// in the same "traySettings" NVS namespace the stock firmware already uses. Purely
// additive: nothing here runs until INTEGRATION.md hook points call it.
#include "bin_layer.h"

int pillsRemaining[11]    = {0};
int binLowThreshold[11]   = {5,5,5,5,5,5,5,5,5,5,5};
int binMaxPerDay[11]      = {20,20,20,20,20,20,20,20,20,20,20};
int binDispensedToday[11] = {0};
int binSensorPin[11]      = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; // set once IR wired

bool binIsEmpty(int bin) { return pillsRemaining[bin] <= 0; }

bool binIsLow(int bin)   { return pillsRemaining[bin] <= binLowThreshold[bin]; }

bool binCapReached(int bin, int want) {
  return binDispensedToday[bin] + want > binMaxPerDay[bin];
}

// Persist just the bulk-count fields for one bin. Uses the existing per-tray key
// scheme (keyName + index) in the "traySettings" namespace so it coexists with the
// stock trayName%d / trayHour%d / … keys.
void savePillCount(int bin) {
  if (bin < 1 || bin > 10) return;
  char key[24];
  preferences.begin("traySettings", false);
  snprintf(key, sizeof(key), "pillsRem%d", bin);
  preferences.putInt(key, pillsRemaining[bin]);
  snprintf(key, sizeof(key), "binLow%d", bin);
  preferences.putInt(key, binLowThreshold[bin]);
  snprintf(key, sizeof(key), "binMax%d", bin);
  preferences.putInt(key, binMaxPerDay[bin]);
  snprintf(key, sizeof(key), "binDisp%d", bin);
  preferences.putInt(key, binDispensedToday[bin]);
  preferences.end();
}

void loadPillCount(int bin) {
  if (bin < 1 || bin > 10) return;
  char key[24];
  preferences.begin("traySettings", true);
  snprintf(key, sizeof(key), "pillsRem%d", bin);
  pillsRemaining[bin]    = preferences.getInt(key, 0);
  snprintf(key, sizeof(key), "binLow%d", bin);
  binLowThreshold[bin]   = preferences.getInt(key, 5);
  snprintf(key, sizeof(key), "binMax%d", bin);
  binMaxPerDay[bin]      = preferences.getInt(key, 20);
  snprintf(key, sizeof(key), "binDisp%d", bin);
  binDispensedToday[bin] = preferences.getInt(key, 0);
  preferences.end();
}

// Call from the existing daily-reset path (where resetHours/resetMin fires) so the
// runaway-guard counters clear once per day. Persists the zeroed counters.
void resetDailyBinCounts() {
  for (int bin = 1; bin <= 10; bin++) {
    binDispensedToday[bin] = 0;
    char key[24];
    preferences.begin("traySettings", false);
    snprintf(key, sizeof(key), "binDisp%d", bin);
    preferences.putInt(key, 0);
    preferences.end();
  }
}
