#include "LPC11xx.h"
#include "gpio.h"

static volatile uint8_t midibuf[3];
static volatile uint8_t midibytesleft = 0;

static inline void handle_midi_command(void) {
  switch (midibuf[0]) {
    case 0x80:  /* note off */
      break;
    case 0x90:  /* note on */
      if (midibuf[1] == 20) {
        // set bit 5 of GPIO1
        GPIOSetValue(PORT1, 5, 1);

        // wait loop
        for (int i = 0; i < 1000000; i++) continue;
        GPIOSetValue(PORT1, 5, 0);
      }
      break;
    case 0xE0:  /* pitch bend */
    default:
      break;
  }
}


void UART_IRQHandler(void) {
  /* get the received byte and clear the interrupt */
  uint8_t byte = LPC_UART->IIR;

  /* command byte */
  if (byte >= 0x80) {
    midibuf[0] = byte;
    switch (byte) {
      case 0x80:  /* note off */
      case 0x90:  /* note on */
      case 0xE0:  /* pitch bend */
        midibytesleft = 2;
        break;
      case 0xC0:  /* program change */
        midibytesleft = 1;
        break;
      case 0xFE:  /* ignore active sense */
        break;
      case 0xFC:  /* stop */
      case 0xFF:  /* reset */
      default:
        midibytesleft = 0;
    }
  } else {
    /* data byte */
    if (midibytesleft > 0) {
      midibuf[3-midibytesleft] = byte;
      midibytesleft--;
    }
    if (midibytesleft <= 0) {
      handle_midi_command();
    }
  }
}
