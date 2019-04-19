#include "LPC11xx.h"			/* LPC11xx Peripheral Registers */
#include "gpio.h"
#include "ssp.h"

namespace SSP {
static LPC_SSP_TypeDef* const LPC_SSP[2] = {LPC_SSP0, LPC_SSP1};
// Statistics of all the interrupts.
volatile uint32_t RX_stats_[2];
volatile uint32_t RX_overrun_stats_[2];
volatile uint32_t RX_timeout_stats_[2];
volatile bool loopback_[2];
volatile bool slave_[2];
Handler handler_[2];
}

void SSP0_IRQHandler() {
  SSP::handler_[SSP::SSP0](SSP::SSP0);
}

void SSP1_IRQHandler() {
  SSP::handler_[SSP::SSP1](SSP::SSP1);
}


void SSP::SetIRQHandler(SSP ssp, Handler handler) {
  handler_[ssp] = handler;
}

void SSP::DefaultIRQHandler(SSP ssp) {
  uint32_t regValue = LPC_SSP[ssp]->MIS;
  // Receive overrun interrupt.
  if (regValue & SSPMIS_RORMIS) {
    RX_overrun_stats_[ssp]++;
    // Clear interrupt.
    LPC_SSP[ssp]->ICR = SSPICR_RORIC;
  }
  // Receive timeout interrupt.
  if (regValue & SSPMIS_RTMIS) {
    RX_timeout_stats_[ssp]++;
    // Clear interrupt.
    LPC_SSP[ssp]->ICR = SSPICR_RTIC;
  }
  // Rx at least half full.
  if (regValue & SSPMIS_RXMIS) {
    // Receive until it's empty.
    RX_stats_[ssp]++;
  }
}

void SSP::IOConfig(SSP ssp, SSP_Config config) {
  if (ssp == SSP0) {
    LPC_SYSCON->PRESETCTRL |= (0x1<<0);
    LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_SSP0;
    // Divided by 2.
    LPC_SYSCON->SSP0CLKDIV = 0x02;
    // SSP IO config.
    if (config.use_MISO) {
      // SSP MISO
      LPC_IOCON->PIO0_8 &= ~0x07;
      LPC_IOCON->PIO0_8  |= 0x01;
    }
    if (config.use_MOSI) {
      // SSP MOSI
      LPC_IOCON->PIO0_9  &= ~0x07;
      LPC_IOCON->PIO0_9 |= 0x01;
    }
    // P0.6 function 2 is SSP clock, need to
    // combined with IOCONSCKLOC register setting.
    LPC_IOCON->SCK_LOC = 0x02;
    LPC_IOCON->PIO0_6 = 0x02;

    if (config.use_CS) {
      // SSP SSEL
      LPC_IOCON->PIO0_2 &= ~0x07;
      LPC_IOCON->PIO0_2 |= 0x01;
    } else {
      // Enable AHB clock to the GPIO domain.
      LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;
      // SSP SSEL is a GPIO pin
      LPC_IOCON->PIO0_2 &= ~0x07;
      // Port0, bit 2 is set to GPIO output and high.
      GPIO::Pin ssel{GPIO::PORT0, 2};
      GPIO::SetDirection(ssel, 1);
      GPIO::SetValue(ssel, 1);
    }
  // SSP1
  } else {
    LPC_SYSCON->PRESETCTRL |= (0x1<<2);
    LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_SSP1;
    // Divided by 2.
    LPC_SYSCON->SSP1CLKDIV = 0x02;
    // SSP I/O config.
    if (config.use_MISO) {
      // SSP MISO
      LPC_IOCON->PIO2_2 &= ~0x07;
      LPC_IOCON->PIO2_2 |= 0x02;
    }
    if (config.use_MOSI) {
      // SSP MOSI
       LPC_IOCON->PIO2_3 &= ~0x07;
       LPC_IOCON->PIO2_3 |= 0x02;
    }
    // SSP CLK
    LPC_IOCON->PIO2_1 &= ~0x07;
    LPC_IOCON->PIO2_1 |= 0x02;

    if (config.use_CS) {
      // SSP SEL
      LPC_IOCON->PIO2_0 &= ~0x07;
      LPC_IOCON->PIO2_0 |= 0x02;
    } else {
      // Enable AHB clock to the GPIO domain.
      LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;
      // SSP SSEL is a GPIO pin.
      LPC_IOCON->PIO2_0 &= ~0x07;
      // Port2, bit 0 is set to GPIO output and high.
      GPIO::Pin ssel{GPIO::PORT0, 2};
      GPIO::SetDirection(ssel, 1);
      GPIO::SetValue(ssel, 1);
    }
  }
}

void SSP::Init(SSP ssp, SSP_Config config) {
  loopback_[ssp] = config.loopback;
  slave_[ssp] = config.slave;
  handler_[ssp] = DefaultIRQHandler;

  IOConfig(SSP0, config);
  // Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15.
  LPC_SSP[ssp]->CR0 = 0x0707;
  SetFrameSize(SSP0, config.frame_size);

  // SSPCPSR clock prescale register, master mode, minimum divisor is 0x02.
  LPC_SSP[ssp]->CPSR = 0x2;

  for (int i = 0; i < FIFOSIZE; i++) {
    // Clear the RxFIFO.
    uint8_t Dummy = LPC_SSP[ssp]->DR;
    UNUSED(Dummy);
  }

  // Enable the SSP Interrupt.
  IRQn_Type irq = static_cast<IRQn_Type>(SSP0_IRQn + ssp);
  NVIC_EnableIRQ(irq);

  // Device select as master, SSP Enabled.
  if (config.loopback) {
    LPC_SSP[ssp]->CR1 = SSPCR1_LBM | SSPCR1_SSE;
  } else {
    if (config.slave) {
    // Slave mode.
      if ( LPC_SSP[ssp]->CR1 & SSPCR1_SSE ) {
        // The slave bit can't be set until SSE bit is zero.
        LPC_SSP[ssp]->CR1 &= ~SSPCR1_SSE;
     }
     // Enable slave bit first.
     LPC_SSP[ssp]->CR1 = SSPCR1_MS;
     // Enable SSP.
     LPC_SSP[ssp]->CR1 |= SSPCR1_SSE;
    } else {
      // Master mode.
      LPC_SSP[ssp]->CR1 = SSPCR1_SSE;
    }
  }
  // Set SSPINMS registers to enable interrupts.
  // Enable all error related interrupts.
  LPC_SSP[ssp]->IMSC = SSPIMSC_RORIM | SSPIMSC_RTIM;
}

void SSP::SetFrameSize(SSP ssp, uint8_t size) {
  // Only frame sizes of 4 to 16 bits are allowed.
  // Fall-back to default value of 8 bits if the siwe is invalid.
  if (size < 4 || size > 16) {
    size = 8;
  }
  // The DDS field [3:0] of CR0 controls the number of bits
  // transferred in each frame.
  LPC_SSP[ssp]->CR0 &= ~0xF;
  LPC_SSP[ssp]->CR0 |= (size - 1) & 0xF;
}

void SSP::Send(SSP ssp, uint8_t *buffer, uint32_t size) {
  for (uint32_t i = 0; i < size; i++) {
    // Move on only if NOT busy and TX FIFO not full.
    while ((LPC_SSP[ssp]->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF) continue;
    LPC_SSP[ssp]->DR = *buffer;
    buffer++;

    if (loopback_[ssp]) {
      // Wait until the Busy bit is cleared.
      while (LPC_SSP[ssp]->SR & SSPSR_BSY) continue;
    } else {
      while ((LPC_SSP[ssp]->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE) continue;
      // Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
      // on MISO. Otherwise, when SSP0Receive() is called, previous data byte
      // is left in the FIFO.
      uint8_t Dummy = LPC_SSP[ssp]->DR;
      UNUSED(Dummy);
    }
  }
}
void SSP::Receive(SSP ssp, uint8_t *buffer, uint32_t size) {
  for (uint32_t i = 0; i < size; i++) {
    // As long as Receive FIFO is not empty, I can always receive.
    // If it's a loopback test, clock is shared for both TX and RX,
    // no need to write dummy byte to get clock to get the data
    // if it's a peer-to-peer communication, SSPDR needs to be written
    // before a read can take place.

    if (loopback_[ssp]) {
      while (!(LPC_SSP[ssp]->SR & SSPSR_RNE)) continue;
    } else {
      if (slave_[ssp]) {
        while (!(LPC_SSP[ssp]->SR & SSPSR_RNE)) continue;
      } else {
        LPC_SSP[ssp]->DR = 0xFF;
        // Wait until the Busy bit is cleared.
        while ((LPC_SSP[ssp]->SR & (SSPSR_BSY|SSPSR_RNE)) != SSPSR_RNE) continue;
      }
    }
    *buffer = LPC_SSP[ssp]->DR;
    buffer++;
  }
}
