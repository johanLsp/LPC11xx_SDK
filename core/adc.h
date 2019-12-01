// Copyright 2019 Johan Lasperas
#ifndef SRC_CORE_ADC_H_
#define SRC_CORE_ADC_H_

#include "LPC11xx.h"

extern "C" void ADC_IRQHandler();

namespace ADC {

typedef void (*Handler)();


void SetIRQHandler(Handler handler);
void DefaultIRQHandler();

void Init();
uint32_t Read(uint8_t channel);
void EnableChannel(uint8_t index);

const uint32_t kMaxValue = 0x3FF;

}  // namespace ADC

#endif  // SRC_CORE_ADC_H_
