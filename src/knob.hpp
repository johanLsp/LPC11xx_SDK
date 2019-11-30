// Copyright 2019 Johan Lasperas
#ifndef SRC_KNOB_HPP_
#define SRC_KNOB_HPP_

#include "core/LPC11xx.h"

class Knob {
 public:
  Knob(uint8_t channel1, uint8_t channel2);
  int Value() { return value_; }
  void ValueChanged();
  void Update();
  void Calibrate() { should_calibrate_ = true; }

  const uint32_t kMaxValue = 1024;

 private:
  uint32_t GetValue(uint32_t phase1, uint32_t phase2);
  void Calibrate(uint32_t phase1, uint32_t phase2);

  const uint8_t kHysteresis = 4;
  uint8_t channel1_;
  uint8_t channel2_;
  uint32_t value_;

  bool should_calibrate_;
  int Vmin_;
  int Vmax_;

  // Calibration parameter
  // Each taper is assumed to behave linearly with saturation
  // limit values 0 and ADC::kMaxValue;
  // Vmax ______________
  //                   /
  //                        Extrapolated
  //                 /
  // ADC::kMaxValue ___
  //               /
  //              /         Actual taper value
  //             /
  //    0 ______/
  //      |    /
  //
  //         /
  //                        Extrapolated
  // Vmin _/
  //
};

#endif  // SRC_KNOB_HPP_
