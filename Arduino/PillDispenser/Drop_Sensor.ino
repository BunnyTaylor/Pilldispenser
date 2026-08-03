// Drop_Sensor.ino — IR break-beam drop verification (safety-critical, brief §5).
//
// One emitter/receiver pair across each bin's drop chute. Exactly one pill should
// break the beam once per stroke. We count debounced beam-break edges inside a
// window and classify: 1 = OK, 0 = jam, >=2 = double-drop.
//
// If a bin has no sensor wired (binSensorPin[bin] < 0) we return DROP_UNVERIFIED so
// the caller can decide policy (default: treat as OK but never for a safety-critical
// med — see INTEGRATION.md / docs/04-safety.md).
#include "bin_layer.h"

static volatile uint32_t s_pulseCount = 0;
static volatile uint32_t s_lastEdgeUs = 0;
static int s_activePin = -1;

// ISR: debounced edge counter for the currently-armed sensor pin.
static void IRAM_ATTR dropISR() {
  uint32_t now = micros();
  if (now - s_lastEdgeUs >= (uint32_t)SENSOR_DEBOUNCE_MS * 1000UL) {
    s_pulseCount++;
    s_lastEdgeUs = now;
  }
}

DropResult waitForDrop(int bin) {
  int pin = (bin >= 1 && bin <= 10) ? binSensorPin[bin] : -1;
  if (pin < 0) return DROP_UNVERIFIED;          // no sensor wired for this bin

  pinMode(pin, SENSOR_ACTIVE_LOW ? INPUT_PULLUP : INPUT);
  s_pulseCount = 0;
  s_lastEdgeUs = micros();
  s_activePin  = pin;
  // Count the edge that corresponds to the beam being broken by a passing pill.
  attachInterrupt(digitalPinToInterrupt(pin),
                  dropISR,
                  SENSOR_ACTIVE_LOW ? FALLING : RISING);

  // Watch the window. Keep the UI + watchdog alive instead of a blind delay().
  uint32_t start = millis();
  while (millis() - start < (uint32_t)SENSOR_WINDOW_MS) {
    lv_timer_handler();
    esp_task_wdt_reset();
    delay(2);
  }

  detachInterrupt(digitalPinToInterrupt(pin));
  s_activePin = -1;

  uint32_t pulses = s_pulseCount;
  if (pulses == 0) return DROP_JAM;
  if (pulses >= 2) return DROP_DOUBLE;
  return DROP_OK;
}
