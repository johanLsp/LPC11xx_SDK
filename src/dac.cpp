// Copyright 2019 Johan Lasperas

#include "dac.hpp"

void DAC::Init() {
  SSP::SSP_Config config;
  config.slave = false;
  config.loopback = false;
  config.use_CS = true;
  config.use_MISO = false;
  config.use_MOSI = true;
  config.frame_size = 16;
  SSP::Init(SSP::SSP0, config);
  Set(DAC_A, 0);
}

void DAC::Set(DAC dac, uint32_t value) {
  uint16_t command;
  uint8_t gain = 1;
  if (value >= kMaxValue) {
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
    UNUSED(Dummy);
  }
}
