// Copyright 2019 Johan Lasperas

#include "core/LPC11xx.h"
#include "core/gpio.h"
#include "core/timer32.h"
#include "core/uart.h"

#include "midi.hpp"
#include "display.hpp"

int main(void) {
  SystemInit();

  // Enable Timer 32 - 0
  init_timer32(TIMER32_0, TIME_INTERVAL);
  enable_timer32(TIMER32_0);

  GPIO::Init();

  LPC_IOCON->PIO0_1  &= ~0x07;
  LPC_IOCON->PIO0_1  |= 0x01;
  LPC_SYSCON->CLKOUTDIV = 0xFF;
  LPC_SYSCON->CLKOUTCLKSEL = 0x03;
  LPC_SYSCON->CLKOUTUEN = 0x00;
  LPC_SYSCON->CLKOUTUEN = 0x01;
  // Set pin 5 of GPIO1 as output.
  GPIO::SetDirection(GPIO::PORT0, 7, 1);
  GPIO::SetValue(GPIO::PORT0, 7, 0);

  Display::Init();
  // 200 * 5ms
  Display::AutoShutdown(1000);
  Midi::Init();
  // Flash LED to notify startup
  GPIO::SetValue(GPIO::PORT0, 7, 1);
  delay32Ms(TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT0, 7, 0);
  delay32Ms(TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT0, 7, 1);
  delay32Ms(TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT0, 7, 0);
  delay32Ms(TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT0, 7, 1);
  delay32Ms(TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT0, 7, 0);

  Display::Print("----");


  char msg[4];

  uint8_t clk_1 = (LPC_SYSCON->SYSAHBCLKDIV >> 8) & 0x000F;
  uint8_t clk_0 = (LPC_SYSCON->SYSAHBCLKDIV >> 0) & 0x000F;

  msg[0] = clk_1 >> 4;
  msg[1] = clk_1 & 0x0F;
  msg[2] = clk_0 >> 4;
  msg[3] = clk_0 & 0x0F;
  Display::Print(msg);

  // Disable unused clocks
  LPC_SYSCON->SYSAHBCLKCTRL &= ~SYSAHBCLKCTRL_IOCON;
  enable_timer32(TIMER32_0);
  LPC_SYSCON->SYSAHBCLKCTRL &= ~SYSAHBCLKCTRL_CT32B0;

  // Enter Sleep-on-Exit mode.
  // MCU will only wake up to handle interrupts
  // and get back to Sleep-on-Exit mode afterward.
  SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
}


void HardFault_Handler(void) {
  // Flash LED to notify startup
  GPIO::SetValue(GPIO::PORT1, 5, 1);
  delay32Ms(TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT1, 5, 0);
  delay32Ms(TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT1, 5, 1);
  delay32Ms(TIMER32_0, 200);
  GPIO::SetValue(GPIO::PORT1, 5, 0);
}

