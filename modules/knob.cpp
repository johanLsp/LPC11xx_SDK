// Copyright 2019 Johan Lasperas

#include "knob.hpp"

#include "core/adc.h"

#define DIST(a, b) (((a) > (b)) ? (a) - (b) : (b) - (a))
#define ABS(a) ((a) > 0 ? (a) : -(a))

Knob::Knob(uint8_t channel1, uint8_t channel2)
: channel1_(channel1), channel2_(channel2), value_(0), Vmin_(0), Vmax_(0),
  should_calibrate_(true) {
  ADC::EnableChannel(channel1);
  ADC::EnableChannel(channel2);
}

int32_t var1;
int32_t var2;

void Knob::ValueChanged() {
  // Do something
}

void Knob::Update() {
  uint32_t phase1 = ADC::Read(channel1_);
  uint32_t phase2 = ADC::Read(channel2_);

  // Update calibration parameters if it wasn't already done.
  // This can be improved by averaging on a few measurements.
  if (should_calibrate_) {
    Calibrate(phase1, phase2);
  }

  uint32_t value = Knob::GetValue(phase1, phase2);
  if (DIST(value, value_) > kHysteresis) {
    value_ = value;
    ValueChanged();
  }
}

uint32_t Knob::GetValue(uint32_t phase1, uint32_t phase2) {
  int32_t value1 = kMaxValue * (phase1 - Vmin_) / (Vmax_ - Vmin_);
  int32_t value2 = kMaxValue * (phase2 - Vmin_) / (Vmax_ - Vmin_);
  // Put value2 back in phase
  value2 = (value2 + kMaxValue / 2) % kMaxValue;

  var1 = value1;
  var2 = value1;

  // If one phase is close to the center, use only the corresponding
  // value as this other value might contain garbage.
  if (DIST(phase1, (Vmax_ - Vmin_) / 2) < (Vmax_ - Vmin_) / 10) return value1;
  var1 = value2;
  var2 = value2;

  if (DIST(phase2, (Vmax_ - Vmin_) / 2) < (Vmax_ - Vmin_) / 10) return value2;
  var1 = value1;
  var2 = value2;
  // Average the two values for higher precision
  return ((value1 + value2) / 2) % kMaxValue;
}

void Knob::Calibrate(uint32_t phase1, uint32_t phase2) {
  // Don't calibrate if one of the phase is too close to the limit,
  // i.e. within 10% from 0 or ADC::kMaxValue
  if (phase1 < ADC::kMaxValue / 10) return;
  if (phase1 > ADC::kMaxValue * 9 / 10) return;
  if (phase2 < ADC::kMaxValue / 10) return;
  if (phase2 > ADC::kMaxValue * 9 / 10) return;

  // Calibrate
  uint32_t phase_delta = DIST(phase1, phase2);
  int32_t Vmin = ADC::kMaxValue / 2 - phase_delta;

  // Fail-safe guard: don't calibrate if the value is odd,
  // i.e :
  //  - Vmin would be positive
  //  - Vmin is too large
  if (Vmin > 0) return;
  if (ABS(Vmin) > 100) return;

  Vmin_ = Vmin;
  Vmax_ = Vmin_ + 2 * phase_delta;
  should_calibrate_ = false;
}
