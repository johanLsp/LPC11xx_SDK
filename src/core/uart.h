#ifndef SRC_CORE_UART_H_
#define SRC_CORE_UART_H_

#include "LPC11xx.h"

#define BUFSIZE         64

extern "C" void UART_IRQHandler(void);
namespace UART {
typedef void (*Handler)();

void SetIRQHandler(Handler handler);
void DefaultIRQHandler();

void Init(uint32_t Baudrate);
void Send(uint8_t *BufferPtr, uint32_t Length);

#define RS485_ENABLED   0

#define IER_RBR         (0x01<<0)
#define IER_THRE        (0x01<<1)
#define IER_RLS         (0x01<<2)
#define IER_ABEO        (0x01<<8)
#define IER_ABTO        (0x01<<9)

#define IIR_PEND        0x01
#define IIR_RLS         0x03
#define IIR_RDA         0x02
#define IIR_CTI         0x06
#define IIR_THRE        0x01
#define IIR_ABEO        (0x01<<8)
#define IIR_ABTO        (0x01<<9)

#define LSR_RDR         (0x01<<0)
#define LSR_OE          (0x01<<1)
#define LSR_PE          (0x01<<2)
#define LSR_FE          (0x01<<3)
#define LSR_BI          (0x01<<4)
#define LSR_THRE        (0x01<<5)
#define LSR_TEMT        (0x01<<6)
#define LSR_RXFE        (0x01<<7)

enum LCR {
  WLS5 = 0,
  WLS6 = 1,
  WLS7 = 2,
  WLS8 = 3,
  SBS2 = 4,
  PE = 8,
  PSODD = 0,
  PSEVEN = 16,
  PS1 = 32,
  PS0 = 48,
  BC = 64,
  DLAB = 128
};
enum FCR {
  FIFOEN = 0x01<<0,
  RXFIFORES = 0x01<<1,
  TXFIFORES = 0x01<<2,
  RXTL0 = 0x00<<6,
  RXTL1 = 0x01<<6,
  RXTL2 = 0x02<<6,
  RXTL3 = 0x03<<6
};
enum RS485 {
  NMMEN = 1,
  RXDIS = 2,
  AADEN = 4,
  SEL = 8,
  DCTRL = 16,
  OINV = 32
};

}  // namespace UART

#endif  // SRC_CORE_UART_H_
