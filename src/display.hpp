#ifndef SRC_DISPLAY_HPP_
#define SRC_DISPLAY_HPP_

#include "core/LPC11xx.h"
#include "core/gpio.h"
#include "core/timer.h"


namespace Display {
struct Pin {
    Pin(uint32_t port, uint32_t pin)
    : port(port), pin(pin) {}
    const uint32_t port;
    const uint32_t pin;
};

void Init();
void Clear();
void Print(const char* msg);
void Print(uint8_t c, uint8_t position);
void Refresh(Timer::Timer timer);
void AutoShutdown(uint32_t shutdown);

}  // namespace Display

#endif  // SRC_DISPLAY_HPP_
