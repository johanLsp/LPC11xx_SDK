#ifndef SRC_CORE_GPIO_H_
#define SRC_CORE_GPIO_H_

#include "LPC11xx.h"

extern "C" void PIOINT0_IRQHandler(void);
extern "C" void PIOINT1_IRQHandler(void);
extern "C" void PIOINT2_IRQHandler(void);
extern "C" void PIOINT3_IRQHandler(void);

namespace GPIO {

enum Port {PORT0 = 0, PORT1, PORT2, PORT3};
struct Pin {
    const Port port;
    const uint32_t pin;
};
typedef void (*Handler)(Pin);

void SetIRQHandler(Pin pin, Handler handler);
void DefaultIRQHandler(Pin pin);
void DispatchInterrupt(Port port);

void Init(Port port);

uint32_t GetValue(Pin pin);
void SetValue(Pin pin, uint32_t value);
void SetDirection(Pin pin, uint32_t direction);

void SetInterrupt(Pin pin, bool level, bool single, bool event);
void EnableInterrupt(Pin pin);
void DisableInterrupt(Pin pin);
void ClearInterrupt(Pin pin);
bool InterruptStatus(Pin pin);

}  // namespace GPIO

#endif  // SRC_CORE_GPIO_H_
