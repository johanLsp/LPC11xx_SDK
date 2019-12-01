// Copyright 2019 Johan Lasperas
#ifndef SRC_SEGMENT_HPP_
#define SRC_SEGMENT_HPP_

#include "core/LPC11xx.h"
#include "core/gpio.h"
#include "core/timer.h"

namespace Segment {

void Init();
void Clear();
void Print(const char* msg);
void Print(uint8_t c, uint8_t position);
void Refresh(Timer::Timer timer);
void AutoShutdown(uint32_t shutdown);

}  // namespace Segment

#endif  // SRC_SEGMENT_HPP_
