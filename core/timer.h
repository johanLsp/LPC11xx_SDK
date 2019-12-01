// Copyright 2019 Johan Lasperas
#ifndef SRC_CORE_TIMER_H_
#define SRC_CORE_TIMER_H_

#include "LPC11xx.h"

extern "C" void TIMER16_0_IRQHandler(void);
extern "C" void TIMER16_1_IRQHandler(void);
extern "C" void TIMER32_0_IRQHandler(void);
extern "C" void TIMER32_1_IRQHandler(void);

namespace Timer {

// For 16-bit timer, make sure that TIME_INTERVAL should be no
// greater than 0xFFFF.
#ifndef TIME_INTERVAL
#define TIME_INTERVAL (SystemCoreClock/1000 - 1)
#endif

enum Timer {TIMER16_0 = 0, TIMER16_1, TIMER32_0, TIMER32_1};
typedef void (*Handler)(Timer);

void SetIRQHandler(Timer timer, Handler handler);
void DefaultIRQHandler(Timer timer);

void Init(Timer timer, uint32_t time_interval);
void Reset(Timer timer);
void Enable(Timer timer);
void Disable(Timer timer);
void ClearInterrupt(Timer timer);
void DelayMs(Timer timer, uint32_t delay_ms);

}  // namespace Timer

#endif  // SRC_CORE_TIMER_H_
