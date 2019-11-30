// Copyright 2019 Johan Lasperas
#include "midi.hpp"

#include "core/uart.h"

namespace Midi {
static volatile uint8_t midibuf[3];
static volatile uint8_t midibytesleft = 0;
static volatile uint8_t currentPos = 0;
static volatile uint8_t num_bytes = 0;
static volatile uint32_t notes_down_[4] = {0, 0, 0, 0};

}  // namespace Midi

void Midi::HandleCommand(uint8_t command, uint8_t note) {
  switch (command & 0xF0) {
    // Note OFF
    case 0x80:
      notes_down_[note/32] &= ~(0x0001 << (note%32));
      break;
    // Note ON
    case 0x90: {
      notes_down_[note/32] |= (0x0001 << (note%32));
      break;
    }
    case 0xE0:  /* pitch bend */
    default:
      break;
  }
}

void Midi::Init() {
  const uint32_t baudrate = 31250;
  UART::Init(baudrate);
  UART::SetIRQHandler(ReceiveHandler);
}

void Midi::ReceiveHandler() {
  // Get the received byte and clear the interrupt.
  uint8_t byte = LPC_UART->RBR;

  // command byte
  if (byte >= 0xA0) {
    midibuf[0] = byte;
    switch (byte) {
      case 0xE0:  // pitch bend
        midibytesleft = 2;
        break;
      case 0xC0:  // program change
        midibytesleft = 1;
        break;
      case 0xFE:  // ignore active sense
        break;
      case 0xFC:  // stop
      case 0xFF:  // reset
      default:
        midibytesleft = 0;
    }
  // Note OFF
  } else if ((byte & 0xF0) == 0x80) {
    midibuf[0] = byte;
    midibytesleft = 2;
  // Note ON
  } else if ((byte & 0xF0) == 0x90) {
    midibuf[0] = byte;
    midibytesleft = 2;
  } else {
    // data byte
    if (midibytesleft > 0) {
      midibuf[3-midibytesleft] = byte;
      midibytesleft--;
    }
    if (midibytesleft == 0) {
      HandleCommand(midibuf[0], midibuf[1]);
    }
  }
}
