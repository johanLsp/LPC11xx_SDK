// Copyright 2019 Johan Lasperas


#include "midi.hpp"

static volatile uint8_t midibuf[3];
static volatile uint8_t midibytesleft = 0;
static volatile uint8_t currentPos = 0;
static volatile uint8_t num_bytes = 0;

void Midi::Print() {
  char msg[4];
  msg[0] = midibuf[0] >> 4;
  msg[1] = midibuf[0] & 0x0F;
  msg[2] = midibuf[1] >> 4;
  msg[3] = midibuf[1] & 0x0F;
  Display::Print(msg);
}

void Midi::HandleCommand(uint8_t command, uint8_t note) {
  Print();
  switch (command) {
    case 0x80:  /* note off */
      if (note == 0x30) {
        GPIO::SetValue(GPIO::PORT0, 7, 0);
      }
      break;
    case 0x90:  /* note on */
      if (note == 0x30) {
        GPIO::SetValue(GPIO::PORT0, 7, 1);
      }
      break;
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
  // get the received byte and clear the interrupt
  //uint8_t IIRValue = LPC_UART->IIR;
  //uint8_t LSRValue = LPC_UART->LSR;
  uint8_t byte = LPC_UART->RBR;

  // command byte
  if (byte >= 0x80) {
    midibuf[0] = byte;
    switch (byte) {
      case 0x80:  // note off
      case 0x90:  // note on
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
