#include "LPC11xx.h"
#include "gpio.h"

volatile uint32_t counters[4] = {0, 0, 0, 0};
volatile uint32_t interrupts[4]  = {0, 0, 0, 0};

void PIOINT0_IRQHandler(void) {
  GPIO::DefaultIRQHandler(GPIO::PORT0);
}

void PIOINT1_IRQHandler(void) {
  GPIO::DefaultIRQHandler(GPIO::PORT1);
}

void PIOINT2_IRQHandler(void) {
  GPIO::DefaultIRQHandler(GPIO::PORT2);
}

void PIOINT3_IRQHandler(void) {
  GPIO::DefaultIRQHandler(GPIO::PORT3);
}

void GPIO::DefaultIRQHandler(uint8_t port) {
  counters[port]++;
  uint32_t regVal = GPIO::IntStatus(port, 1);
  if (regVal) {
    interrupts[port]++;
    GPIO::IntClear(port, 1);
  }
  return;
}

void GPIO::Init() {
  // enable IOCON clock
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);
  // Enable AHB clock to the GPIO domain.
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

  // Set up NVIC when I/O pins are configured as external interrupts.
  NVIC_EnableIRQ(EINT0_IRQn);
  NVIC_EnableIRQ(EINT1_IRQn);
  NVIC_EnableIRQ(EINT2_IRQn);
  NVIC_EnableIRQ(EINT3_IRQn);
  return;
}


uint32_t GPIO::GetValue(uint32_t port, uint32_t bit) {
  uint32_t regVal = 0;
  if (bit < 0x20) {
    if (LPC_GPIO[port]->DATA & (0x1 << bit)) {
      regVal = 1;
    }
  } else if (bit == 0xFF) {
    regVal = LPC_GPIO[port]->DATA;
  }
  return regVal;
}

void GPIO::SetValue(uint32_t port, uint32_t bit, uint32_t value) {
  LPC_GPIO[port]->MASKED_ACCESS[(1 << bit)] = (value << bit);
}

void GPIO::SetDirection(uint32_t port, uint32_t bit, uint32_t direction) {
  if (direction) {
    LPC_GPIO[port]->DIR |= 1 << bit;
  } else {
    LPC_GPIO[port]->DIR &= ~(1 << bit);
  }
}

void GPIO::SetInterrupt(uint32_t port, uint32_t bit, uint32_t sense,
                        uint32_t single, uint32_t event) {
  if (sense == 0) {
    LPC_GPIO[port]->IS &= ~(0x1 << bit);
    // single or double only applies when sense is 0(edge trigger).
    if (single == 0) {
      LPC_GPIO[port]->IBE &= ~(0x1 << bit);
    } else {
      LPC_GPIO[port]->IBE |= (0x1 << bit);
    }
  } else {
    LPC_GPIO[port]->IS |= (0x1 << bit);
    if (event == 0) {
      LPC_GPIO[port]->IEV &= ~(0x1 << bit);
    } else {
      LPC_GPIO[port]->IEV |= (0x1 << bit);
    }
  }
  return;
}

void GPIO::IntEnable(uint32_t port, uint32_t bit) {
  LPC_GPIO[port]->IE |= (0x1 << bit);
}

void GPIO::IntDisable(uint32_t port, uint32_t bit) {
  LPC_GPIO[port]->IE &= ~(0x1 << bit);
}

uint32_t GPIO::IntStatus(uint32_t port, uint32_t bit) {
  uint32_t regVal = 0;
    if (LPC_GPIO[port]->MIS & (0x1 << bit)) {
      regVal = 1;
    }
  return regVal;
}

void GPIO::IntClear(uint32_t port, uint32_t bit) {
  LPC_GPIO[port]->IC |= (0x1 << bit);
}
