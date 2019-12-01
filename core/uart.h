// Copyright 2019 Johan Lasperas
#ifndef SRC_CORE_UART_H_
#define SRC_CORE_UART_H_

#include "LPC11xx.h"

#define BUFSIZE         64

extern "C" void UART_IRQHandler(void);
namespace UART {
typedef void (*Handler)();

enum Parity {NONE = -1, ODD = 0, EVEN = 1, FORCE1 = 2, FORCE0 = 3};
enum Bits {WLS5 = 0, WLS6 = 1, WLS7 = 2, WLS8 = 3};
enum StopBits {SB1 = 0, SB2 = 1};
struct Config {
  uint32_t baudrate;
  Bits bits;
  Parity parity;
  StopBits stopbits;
};

void SetIRQHandler(Handler handler);
void DefaultIRQHandler();

void Init(const Config& config);
void Send(const uint8_t* buffer, uint32_t length);
// Convenience function to pass a null-terminated string.
void Send(const char* buffer);

namespace IER {
  enum IER {
    RBR = 1,
    THRE = 2,
    RLS = 4,
    ABEO = 256,
    ABTO = 512
  };
}
namespace IIR {
enum IIR {
  PEND = 1,
  RLS = 3, RDA = 2, CTI = 6, THRE = 1,
  ABEO = 256,
  ABTO = 512
};
}
namespace LSR {
enum LSR {
  RDR = 1,
  OE = 2,
  PE = 4,
  FE = 8,
  BI = 16,
  THRE = 32,
  TEMT = 64,
  RXFE = 128
};
}  // namespace LSR
namespace LCR {
enum LCR {
  WLS5 = 0, WLS6 = 1, WLS7 = 2, WLS8 = 3,
  SBS2 = 4,
  PE = 8,
  PSODD = 0, PSEVEN = 16, PS1 = 32, PS0 = 48,
  BC = 64,
  DLAB = 128
};
}
namespace FCR {
enum FCR {
  FIFOEN = 0x01<<0,
  RXFIFORES = 0x01<<1,
  TXFIFORES = 0x01<<2,
  RXTL0 = 0x00<<6, RXTL1 = 0x01<<6, RXTL2 = 0x02<<6, RXTL3 = 0x03<<6
};
}
namespace RS485 {
enum RS485 {
  NMMEN = 1,
  RXDIS = 2,
  AADEN = 4,
  SEL = 8,
  DCTRL = 16,
  OINV = 32
};
}

}  // namespace UART

#endif  // SRC_CORE_UART_H_
