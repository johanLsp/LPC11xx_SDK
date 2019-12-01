#include "gpio.h"

LPC_GPIO_TypeDef* const GPIO::LPC_GPIO[4] = {LPC_GPIO0, LPC_GPIO1,
                                              LPC_GPIO2, LPC_GPIO3};

GPIO::Handler GPIO::handler_[4][12] = {GPIO::DefaultIRQHandler};

void PIOINT0_IRQHandler() {
  GPIO::DispatchInterrupt(GPIO::PORT0);
}

void PIOINT1_IRQHandler() {
  GPIO::DispatchInterrupt(GPIO::PORT1);
}

void PIOINT2_IRQHandler() {
  GPIO::DispatchInterrupt(GPIO::PORT2);
}

void PIOINT3_IRQHandler() {
  GPIO::DispatchInterrupt(GPIO::PORT3);
}

void GPIO::DispatchInterrupt(Port port) {
  for (uint32_t i = 0; i < 12; i++) {
    Pin pin{port, i};
    if (InterruptStatus(pin))
    handler_[port][i](pin);
  }
}

void GPIO::SetIRQHandler(Pin pin, Handler handler) {
  handler_[pin.port][pin.pin] = handler;
}

void GPIO::DefaultIRQHandler(Pin pin) {
  ClearInterrupt(pin);
}

GPIO::GPIO(Pin pin, Direction direction)
: pin_(pin) {
  SetDirection(direction);
  Off();
}

void GPIO::SetDirection(Direction direction) {
  if (direction) {
    LPC_GPIO[pin_.port]->DIR |= 1 << pin_.pin;
  } else {
    LPC_GPIO[pin_.port]->DIR &= ~(1 << pin_.pin);
  }
}

void GPIO::Init(Port port) {
  // enable IOCON clock
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_IOCON;
  // Enable AHB clock to the GPIO domain.
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;
  // Set up NVIC when I/O pins are configured as external interrupts.
  // Enable the TIMER0 Interrupt
  IRQn_Type irq = static_cast<IRQn_Type>(EINT0_IRQn + port);

  NVIC_EnableIRQ(irq);
  return;
}

uint32_t GPIO::Read() {
  uint32_t regVal = 0;
  if (pin_.pin < 0x20) {
    if (LPC_GPIO[pin_.port]->DATA & (0x1 << pin_.pin)) {
      regVal = 1;
    }
  } else if (pin_.pin == 0xFF) {
    regVal = LPC_GPIO[pin_.port]->DATA;
  }
  return regVal;
}

void GPIO::On() {
  value_ = 1;
  LPC_GPIO[pin_.port]->MASKED_ACCESS[(1 << pin_.pin)] = (1 << pin_.pin);
}

void GPIO::Off() {
  value_ = 0;
  LPC_GPIO[pin_.port]->MASKED_ACCESS[(1 << pin_.pin)] = (0 << pin_.pin);
}

void GPIO::Toggle() {
  value_ = value_ ? 0 : 1;
  LPC_GPIO[pin_.port]->MASKED_ACCESS[(1 << pin_.pin)] = (value_ << pin_.pin);
}

void GPIO::SetInterrupt(Pin pin, bool level, bool single, bool event) {
  Port port = pin.port;
  uint32_t bit = pin.pin;

  // Detect level
  if (level) {
    LPC_GPIO[port]->IS |= (0x1 << bit);
  // Detec edge
  } else {
    LPC_GPIO[port]->IS &= ~(0x1 << bit);
    // One edge
    if (single) {
      LPC_GPIO[port]->IBE &= ~(0x1 << bit);
    // Both edges
    } else {
      LPC_GPIO[port]->IBE |= (0x1 << bit);
    }
  }

  // Detect level high or rising edge.
  if (event) {
    LPC_GPIO[port]->IEV |= (0x1 << bit);
  // Detect level low or falling edge.
  } else {
    LPC_GPIO[port]->IEV &= ~(0x1 << bit);
  }

  SetIRQHandler(pin, DefaultIRQHandler);
  EnableInterrupt(pin);
}

void GPIO::EnableInterrupt(Pin pin) {
  LPC_GPIO[pin.port]->IE |= (0x1 << pin.pin);
}

void GPIO::DisableInterrupt(Pin pin) {
  LPC_GPIO[pin.port]->IE &= ~(0x1 << pin.pin);
}

bool GPIO::InterruptStatus(Pin pin) {
    if (LPC_GPIO[pin.port]->MIS & (0x1 << pin.pin)) {
      return true;
    }
  return false;
}

void GPIO::ClearInterrupt(Pin pin) {
  LPC_GPIO[pin.port]->IC |= (0x1 << pin.pin);
}
