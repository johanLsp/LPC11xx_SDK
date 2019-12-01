#ifndef SRC_CORE_GPIO_H_
#define SRC_CORE_GPIO_H_

#include "LPC11xx.h"

extern "C" void PIOINT0_IRQHandler(void);
extern "C" void PIOINT1_IRQHandler(void);
extern "C" void PIOINT2_IRQHandler(void);
extern "C" void PIOINT3_IRQHandler(void);

class GPIO {
 public:
  enum Port {PORT0 = 0, PORT1, PORT2, PORT3};
  enum Direction {INPUT = 0, OUTPUT = 1};
  struct Pin {
      const Port port;
      const uint32_t pin;
  };
  typedef void (*Handler)(Pin);

  static void Init(Port port);

  GPIO(Pin pin, Direction direction);

  uint32_t Read();
  void Toggle();
  void On();
  void Off();

  static void DispatchInterrupt(Port port);
  static void SetIRQHandler(Pin pin, Handler handler);
  static void DefaultIRQHandler(Pin pin);
  static bool InterruptStatus(Pin pin);
  static void SetInterrupt(Pin pin, bool level, bool single, bool event);
  static void EnableInterrupt(Pin pin);
  static void DisableInterrupt(Pin pin);


 private:
  static LPC_GPIO_TypeDef* const LPC_GPIO[4];
  static void ClearInterrupt(Pin pin);
  static Handler handler_[4][12];

  void SetDirection(Direction direction);

  // GPIO has up to 4 ports, 12 pins per port.
  Pin pin_;
  uint8_t value_;
};

#endif  // SRC_CORE_GPIO_H_
