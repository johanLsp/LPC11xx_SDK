// Copyright 2019 Johan Lasperas
#ifndef SDK_CORE_GPIO_H_
#define SDK_CORE_GPIO_H_

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
      bool operator==(const Pin& rhs) {
        return port == rhs.port && pin == rhs.pin;
      }
  };

  struct Trigger {
    enum Type {LEVEL, EDGE} type;
    enum Edge {RISING, FALLING, BOTH} edge;
  };
  typedef void (*Handler)(const GPIO& gpio);

  GPIO(Pin pin, Direction direction);

  uint32_t Read();
  void Toggle();
  void On();
  void Off();

  // Interrupt-related methods
  void SetInterrupt(Trigger trigger, Handler handler);
  void EnableInterrupt();
  void DisableInterrupt();

  static void DispatchInterrupt(Port port);

 private:
  static LPC_GPIO_TypeDef* const LPC_GPIO[4];
  static Handler handler_[4][12];
  static GPIO* gpio_[4][12];

  void SetDirection(Direction direction);
  void ClearInterrupt();
  void SetIRQHandler(Handler handler);
  bool InterruptStatus();

  // GPIO has up to 4 ports, 12 pins per port.
  Pin pin_;
  uint8_t value_;
};

#endif  // SDK_CORE_GPIO_H_
