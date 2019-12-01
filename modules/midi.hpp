// Copyright 2019 Johan Lasperas
#ifndef SRC_MIDI_HPP_
#define SRC_MIDI_HPP_

#include "core/LPC11xx.h"

namespace Midi {
void Init();
void ReceiveHandler();
void HandleCommand(uint8_t command, uint8_t note);

}  // namespace Midi

#endif  // SRC_MIDI_HPP_
