// Copyright 2019 Johan Lasperas

#include "core/LPC11xx.h"
#include "core/gpio.h"
#include "core/timer.h"
#include "core/uart.h"

#include "encoder.hpp"
#include "dac.hpp"
#include "segment.hpp"
#include "midi.hpp"

int main(void) {
  SystemInit();

  // Enable Timer 32 - 0
  Timer::Init(Timer::TIMER32_0, TIME_INTERVAL);
  Timer::Enable(Timer::TIMER32_0);

  GPIO::Init(GPIO::PORT0);
  GPIO::Init(GPIO::PORT1);

  Segment::Init();
  // 200 * 5ms
  Segment::AutoShutdown(1000);
  Midi::Init();
  DAC::Init();
  Encoder::Init();

  Segment::Print("----");
  Timer::DelayMs(Timer::TIMER32_0, 100);

  // Disable unused clocks
  LPC_SYSCON->SYSAHBCLKCTRL &= ~SYSAHBCLKCTRL_IOCON;
  Disable(Timer::TIMER32_0);
  LPC_SYSCON->SYSAHBCLKCTRL &= ~SYSAHBCLKCTRL_CT32B0;

  // Enter Sleep-on-Exit mode.
  // MCU will only wake up to handle interrupts
  // and get back to Sleep-on-Exit mode afterward.
  SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
}


void HardFault_Handler(void) {
  Segment::Print("EEEE");
}

