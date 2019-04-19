#ifndef SRC_MIDI_HPP_
#define SRC_MIDI_HPP_

#include "core/LPC11xx.h"
#include "core/gpio.h"
#include "core/uart.h"
#include "display.hpp"

namespace Midi {
void Init();
void ReceiveHandler();
void Print();
void HandleCommand(uint8_t command, uint8_t note);

}  // namespace Midi

#endif  // SRC_MIDI_HPP_
