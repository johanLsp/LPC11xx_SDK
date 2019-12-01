// Copyright 2019 Johan Lasperas
#include "adc.h"

#include "core/gpio.h"

namespace ADC {
uint16_t value_[8];
Handler handler_;

struct Channel {
  volatile  uint32_t* const reg;
  const GPIO::Pin pin;
};

const Channel channels_[] = {
  {&LPC_IOCON->R_PIO0_11, {GPIO::PORT0, 11}},
  {&LPC_IOCON->R_PIO1_0, {GPIO::PORT1, 0}},
  {&LPC_IOCON->R_PIO1_1, {GPIO::PORT1, 1}},
  {&LPC_IOCON->R_PIO1_2, {GPIO::PORT1, 2}},
  {&LPC_IOCON->SWDIO_PIO1_3, {GPIO::PORT1, 3}},
  {&LPC_IOCON->PIO1_4, {GPIO::PORT1, 4}},
  {&LPC_IOCON->PIO1_10, {GPIO::PORT1, 10}},
  {&LPC_IOCON->PIO1_11, {GPIO::PORT1, 11}}
};
}  // namespace ADC

void ADC_IRQHandler() {
  ADC::handler_();
}

void ADC::SetIRQHandler(Handler handler) {
  handler_ = handler;
}

void ADC::DefaultIRQHandler() {
}

void ADC::Init() {
  handler_ = DefaultIRQHandler;
    // Power up ADC
  LPC_SYSCON->PDRUNCFG &= ~(0x1 << 4);

  // Enable the clock for the ADC and IOCON domain
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_IOCON;
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_ADC;

  // Set clock close to 4.5MHz from PCLK
  uint32_t Fdiv = (SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/4500000 - 1;

  LPC_ADC->CR = 0;
  LPC_ADC->CR |= Fdiv << 8;
}

void ADC::EnableChannel(uint8_t index) {
  Channel channel = channels_[index];
  GPIO::SetDirection(channel.pin, 1);
  // Enable AD functions
  *channel.reg &= ~0x07;
  *channel.reg  |= index > 4 ? 0x01 : 0x02;
  // Pulldown/up inactive
  *channel.reg  &= ~(0x03 << 3);
  // Set Analog mode
  *channel.reg  &= ~(0x01 << 7);
  value_[index] = 0;
}

uint32_t ADC::Read(uint8_t channel) {
  LPC_ADC->CR &= ~0x000000FF;
  LPC_ADC->CR |= 0x01 << channel;
  LPC_ADC->CR |= 0x00000001 << 24;

  uint32_t value;
  while (true) {
    value = LPC_ADC->DR[channel];
    if (value & (0x1U << 31)) break;
  }

  LPC_ADC->CR &= ~(0x7 << 24);
  LPC_ADC->CR &= ~(0x01 << channel);

  if (value & (0x1 << 30)) return 0;

  return (value >> 6) % kMaxValue;
}
