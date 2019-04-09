#ifndef SRC_CORE_GPIO_H_
#define SRC_CORE_GPIO_H_


extern "C" void PIOINT0_IRQHandler(void);
extern "C" void PIOINT1_IRQHandler(void);
extern "C" void PIOINT2_IRQHandler(void);
extern "C" void PIOINT3_IRQHandler(void);

namespace GPIO {
enum Port {PORT0 = 0, PORT1, PORT2, PORT3};
static LPC_GPIO_TypeDef (* const LPC_GPIO[4]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3 };

void Init();
LPC_GPIO_TypeDef* GetGPIO(uint8_t port);
void DefaultIRQHandler(uint8_t port);
void SetInterrupt(uint32_t port, uint32_t bit, uint32_t sense,
                  uint32_t single, uint32_t event);
void IntEnable(uint32_t port, uint32_t bit);
void IntDisable(uint32_t port, uint32_t bit);
uint32_t IntStatus(uint32_t port, uint32_t bit);
void IntClear(uint32_t port, uint32_t bit);
uint32_t GetValue(uint32_t port, uint32_t bit);
void SetValue(uint32_t port, uint32_t bit, uint32_t value);
void SetDirection(uint32_t port, uint32_t bit, uint32_t direction);

}  // namespace GPIO

#endif  // SRC_CORE_GPIO_H_
