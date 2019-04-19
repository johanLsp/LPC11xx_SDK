#ifndef SRC_ENCODER_HPP_
#define SRC_ENCODER_HPP_

#include "core/LPC11xx.h"
#include "core/gpio.h"

namespace Encoder {

void Init();
// Events are debounced using table storing valid transitions
// for each previous stored state (clk,data) to the new state.
void EventHandler(GPIO::Pin pin);
void ValueChanged();
uint32_t GetState();
}  // namespace Encoder

#endif  // SRC_ENCODER_HPP_
