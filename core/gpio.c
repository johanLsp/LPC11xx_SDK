#include "gpio.h"

LPC_GPIO_TypeDef* const GPIO::LPC_GPIO[4] = {LPC_GPIO0, LPC_GPIO1,
                                              LPC_GPIO2, LPC_GPIO3};

GPIO::Handler GPIO::handler_[4][12];
GPIO* GPIO::gpio_[4][12];

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
    GPIO* gpio = gpio_[port][i];
    if (gpio && gpio->InterruptStatus()) {
      handler_[port][i](*gpio);
      gpio->ClearInterrupt();
    }
  }
}

void GPIO::SetIRQHandler(Handler handler) {
  handler_[pin_.port][pin_.pin] = handler;
  gpio_[pin_.port][pin_.pin] = this;
}

GPIO::GPIO(Pin pin, Direction direction)
: pin_(pin) {
  // Enable IOCON clock
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_IOCON;
  // Enable AHB clock to the GPIO domain.
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;

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

void GPIO::SetInterrupt(Trigger trigger, Handler handler) {
  Port port = pin_.port;
  uint32_t pin = pin_.pin;

  // Set up NVIC when I/O pins are configured as external interrupts.
  IRQn_Type irq = static_cast<IRQn_Type>(EINT0_IRQn + port);
  NVIC_EnableIRQ(irq);

  // Detect level
  if (trigger.type == Trigger::LEVEL) {
    LPC_GPIO[port]->IS |= (0x1 << pin);
  // Detect edge
  } else {
    LPC_GPIO[port]->IS &= ~(0x1 << pin);
  }

  // Both edges
  if (trigger.edge == Trigger::BOTH) {
    LPC_GPIO[port]->IBE |= (0x1 << pin);
  // Rising edge
  } else if (trigger.edge == Trigger::RISING) {
    LPC_GPIO[port]->IBE &= ~(0x1 << pin);
    LPC_GPIO[port]->IEV |= (0x1 << pin);
  // Falling edge
  } else {
    LPC_GPIO[port]->IBE &= ~(0x1 << pin);
    LPC_GPIO[port]->IEV &= ~(0x1 << pin);
  }
  SetIRQHandler(handler);
  EnableInterrupt();
}

void GPIO::EnableInterrupt() {
  LPC_GPIO[pin_.port]->IE |= (0x1 << pin_.pin);
}

void GPIO::DisableInterrupt() {
  LPC_GPIO[pin_.port]->IE &= ~(0x1 << pin_.pin);
}

bool GPIO::InterruptStatus() {
    if (LPC_GPIO[pin_.port]->MIS & (0x1 << pin_.pin)) {
      return true;
    }
  return false;
}

void GPIO::ClearInterrupt() {
  LPC_GPIO[pin_.port]->IC |= (0x1 << pin_.pin);
}
