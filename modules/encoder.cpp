// Copyright 2019 Johan Lasperas
#include "encoder.hpp"

namespace Encoder {
const GPIO::Pin clock_ = {GPIO::PORT0, 3};
const GPIO::Pin data_ = {GPIO::PORT0, 4};
const GPIO::Pin switch_ = {GPIO::PORT0, 7};
volatile uint32_t value_ = 0;

const bool valid_transitions_[16] = {false, true, true, false,
                                     true, false, false, true,
                                     true, false, false, true,
                                     false, true, true, false};
volatile uint8_t previous_state_ = 0;
volatile uint16_t previous_transition_ = 0;
}  // namespace Encoder

void Encoder::Init() {
  GPIO::SetDirection(clock_, 0);
  GPIO::SetDirection(data_, 0);
  GPIO::SetDirection(switch_, 0);

  GPIO::SetInterrupt(clock_, false, false, true);
  GPIO::SetInterrupt(data_, false, false, true);
  GPIO::SetInterrupt(switch_, false, true, true);

  GPIO::SetIRQHandler(clock_, Turned);
  GPIO::SetIRQHandler(data_, Turned);
  GPIO::SetIRQHandler(switch_, Clicked);

  previous_state_ = GetState();
}

uint32_t Encoder::GetState() {
  return 2*GPIO::GetValue(clock_) + GPIO::GetValue(data_);
}

void Encoder::Clicked(GPIO::Pin pin) {
  // Do something
}

void Encoder::Turned(GPIO::Pin pin) {
  int state = GetState();
  int transition = 4*previous_state_ + state;
  if (valid_transitions_[transition]) {
    if (previous_transition_ == 0x2 && transition == 0xB) {
      value_--;
      ValueChanged();
    } else if (previous_transition_ == 0x1 && transition == 0x7) {
      value_++;
      ValueChanged();
    }
    previous_state_ = state;
    previous_transition_ = transition;
  }
}

void Encoder::ValueChanged() {
  // Do something
}
