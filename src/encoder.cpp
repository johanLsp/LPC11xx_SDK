// Copyright 2019 Johan Lasperas

#include "encoder.hpp"
#include "dac.hpp"
#include "display.hpp"


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

  GPIO::SetIRQHandler(clock_, EventHandler);
  GPIO::SetIRQHandler(data_, EventHandler);
  GPIO::SetIRQHandler(switch_, EventHandler);

  previous_state_ = GetState();
}

uint32_t Encoder::GetState() {
  return 2*GPIO::GetValue(clock_) + GPIO::GetValue(data_);
}

void Encoder::EventHandler(GPIO::Pin pin) {
  if (GPIO::InterruptStatus(clock_) || GPIO::InterruptStatus(data_)) {
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
  } else if (GPIO::InterruptStatus(switch_)) {
    Display::Print("----");
  }
  GPIO::ClearInterrupt(pin);
}

void Encoder::ValueChanged() {
  char msg[4];
  msg[3] = value_ % 10;
  msg[2] = (value_ / 10) % 10;
  msg[1] = (value_ / 100) % 10;
  msg[0] = (value_ / 1000) % 10;
  Display::Print(msg);
  if (value_ < 16) {
    DAC::Set(DAC::DAC_A, value_ * 256);
  } else if (value_ == 16) {
    DAC::Set(DAC::DAC_A, 4095);
  }
}
