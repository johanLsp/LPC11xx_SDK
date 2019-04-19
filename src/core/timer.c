#include "timer.h"

namespace Timer {
static  LPC_TMR_TypeDef* const LPC_TMR[4] = {LPC_TMR16B0, LPC_TMR16B1,
                                            LPC_TMR32B0, LPC_TMR32B1};
volatile uint32_t counter_[4];
volatile uint32_t capture_[4];
volatile uint32_t period_[4];
Handler handler_[4];
}

void TIMER16_0_IRQHandler() {
  Timer::handler_[Timer::TIMER16_0](Timer::TIMER16_0);
}
void TIMER16_1_IRQHandler() {
  Timer::handler_[Timer::TIMER16_1](Timer::TIMER16_1);
}
void TIMER32_0_IRQHandler() {
  Timer::handler_[Timer::TIMER32_0](Timer::TIMER32_0);
}
void TIMER32_1_IRQHandler() {
  Timer::handler_[Timer::TIMER32_1](Timer::TIMER32_1);
}

void Timer::DefaultIRQHandler(Timer timer) {
  ClearInterrupt(timer);
}

void Timer::SetIRQHandler(Timer timer, Handler handler) {
  handler_[timer] = handler;
}

void Timer::Init(Timer timer, uint32_t timer_interval) {
  counter_[timer] = 0;
  capture_[timer] = 0;
  handler_[timer] = DefaultIRQHandler;
  LPC_SYSCON->SYSAHBCLKCTRL |= (SYSAHBCLKCTRL_CT16B0 << timer);

  LPC_TMR[timer]->MR0 = timer_interval;

  LPC_TMR[timer]->CCR = (0x1<<0)|(0x1<<2);

  // Interrupt and Reset on MR0 and MR1
  LPC_TMR[timer]->MCR = 3;
  // Enable the TIMER0 Interrupt
  IRQn_Type irq = static_cast<IRQn_Type>(TIMER_16_0_IRQn + timer);
  NVIC_EnableIRQ(irq);
}

void Timer::Enable(Timer timer) {
  LPC_TMR[timer]->TCR = 1;
}

void Timer::Disable(Timer timer) {
  LPC_TMR[timer]->TCR = 0;
}

void Timer::Reset(Timer timer) {
  uint32_t regVal = LPC_TMR[timer]->TCR;
  regVal |= 0x02;
  LPC_TMR[timer]->TCR = regVal;
}

void Timer::ClearInterrupt(Timer timer) {
  if (LPC_TMR[timer]->IR & 0x1) {
    // clear interrupt flag
    LPC_TMR[timer]->IR = 1;
    counter_[timer]++;
  }
  if (LPC_TMR[timer]->IR & (0x1 << 4)) {
    // clear interrupt flag
    LPC_TMR[timer]->IR = 0x1 << 4;
    capture_[timer]++;
  }
}

void Timer::DelayMs(Timer timer, uint32_t delay_ms) {
  // Reset timer.
  LPC_TMR[timer]->TCR = 0x02;
  // Set prescaler to zero.
  LPC_TMR[timer]->PR  = 0x0;
  LPC_TMR[timer]->MR0 = delay_ms * (SystemCoreClock / 1000);
  // Reset all interrupts.
  LPC_TMR[timer]->IR  = 0xff;
  // Stop timer match.
  LPC_TMR[timer]->MCR = 0x04;
  // Start timer.
  LPC_TMR[timer]->TCR = 0x01;
  // Wait until delay time has elapsed.
  while (LPC_TMR[timer]->TCR & 0x01) continue;
}
