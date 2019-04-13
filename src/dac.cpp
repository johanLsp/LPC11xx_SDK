// Copyright 2019 Johan Lasperas

#include "dac.hpp"
#include "display.hpp"

namespace DAC {
}

void DAC::Init() {
  // IO Config
  LPC_SYSCON->PRESETCTRL |= (0x1<<0);
  LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<11);
  // Divided by2.
  LPC_SYSCON->SSP0CLKDIV = 0x02;
  // SSP MISO
  // LPC_IOCON->PIO0_8 &= ~0x07;
  // LPC_IOCON->PIO0_8 |= 0x01;
  // SSP MOSI
  LPC_IOCON->PIO0_9 &= ~0x07;
  LPC_IOCON->PIO0_9 |= 0x01;
  // SSP SCK
  LPC_IOCON->SCK_LOC = 0x02;
  LPC_IOCON->PIO0_6 = 0x02;

  // SSP CS
  LPC_IOCON->PIO0_2 &= ~0x07;
  LPC_IOCON->PIO0_2 |= 0x01;

  // Set DSS data to 16-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15
  LPC_SSP0->CR0 = 0x070F;
  // SSPCPSR clock prescale register, master mode, minimum divisor is 0x02
  LPC_SSP0->CPSR = 0x2;

  // Enable the SSP Interrupt
  NVIC_EnableIRQ(SSP0_IRQn);

  // Device select as master, SSP Enabled
  // Master mode
  LPC_SSP0->CR1 = SSPCR1_SSE;
  // Set SSPINMS registers to enable interrupts
  // enable all error related interrupts
  LPC_SSP0->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;
  Set(DAC_A, 0);
}

void DAC::Set(DAC dac, uint32_t value) {
  uint16_t command;
  uint8_t gain = 1;
  if (value >= 4096) {
    value /= 2;
    gain = 0;
  }

  uint8_t shutdown = 1;
  if (value == 0) {
    shutdown = 0;
  }

  command = value;
  // Select DAC A:0, B:1
  command |= dac << 15;
  // Select gain 1x:1, 2x:0
  command |= gain << 13;
  // Output Off:0, On:1
  command |= shutdown << 12;

  Send(&command, 1);
}


void DAC::Send(uint16_t* buffer, uint32_t size) {
  for (uint32_t i = 0; i < size; i++) {
    // Move on only if NOT busy and TX FIFO not full.
    while ((LPC_SSP0->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF) continue;
    LPC_SSP0->DR = *buffer;
    buffer++;
    while ((LPC_SSP0->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE) continue;
    // Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
    // on MISO. Otherwise, when SSP0Receive() is called, previous data byte
    // is left in the FIFO.
    uint8_t Dummy = LPC_SSP0->DR;
  }
}
