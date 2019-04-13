// Copyright 2019 Johan Lasperas

#include "core/LPC11xx.h"
#include "core/gpio.h"
#include "core/timer.h"
#include "core/uart.h"

#include "display.hpp"
#include "midi.hpp"

int main(void) {
  SystemInit();

  // Enable Timer 32 - 0
  Timer::Init(Timer::TIMER32_0, TIME_INTERVAL);
  Timer::Enable(Timer::TIMER32_0);

  GPIO::Init();

  // Set pin 5 of GPIO1 as output.
  GPIO::SetDirection(GPIO::PORT0, 7, 1);
  GPIO::SetValue(GPIO::PORT0, 7, 0);

  Display::Init();
  // 200 * 5ms
  Display::AutoShutdown(1000);
  Midi::Init();
  // Flash LED to notify startup
  GPIO::SetValue(GPIO::PORT0, 7, 1);
  Timer::DelayMs(Timer::TIMER32_0, 100);
  GPIO::SetValue(GPIO::PORT0, 7, 0);
  Timer::DelayMs(Timer::TIMER32_0, 100);
  GPIO::SetValue(GPIO::PORT0, 7, 1);
  Timer::DelayMs(Timer::TIMER32_0, 100);
  GPIO::SetValue(GPIO::PORT0, 7, 0);
  Timer::DelayMs(Timer::TIMER32_0, 100);
  GPIO::SetValue(GPIO::PORT0, 7, 1);
  Timer::DelayMs(Timer::TIMER32_0, 100);
  GPIO::SetValue(GPIO::PORT0, 7, 0);

  Display::Print("----");
  Timer::DelayMs(Timer::TIMER32_0, 100);



  // Disable unused clocks
  LPC_SYSCON->SYSAHBCLKCTRL &= ~SYSAHBCLKCTRL_IOCON;
  Enable(Timer::TIMER32_0);
  LPC_SYSCON->SYSAHBCLKCTRL &= ~SYSAHBCLKCTRL_CT32B0;

  // Enter Sleep-on-Exit mode.
  // MCU will only wake up to handle interrupts
  // and get back to Sleep-on-Exit mode afterward.
  SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
}


void HardFault_Handler(void) {
  // Flash LED to notify startup
  GPIO::SetValue(GPIO::PORT1, 5, 1);
  Timer::DelayMs(Timer::TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT1, 5, 0);
  Timer::DelayMs(Timer::TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT1, 5, 1);
  Timer::DelayMs(Timer::TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT1, 5, 0);
}

