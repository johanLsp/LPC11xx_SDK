/****************************************************************************
 *   $Id:: uart.h 8216 2011-10-06 17:50:47Z usb00423                        $
 *   Project: NXP LPC11xx software example
 *
 *   Description:
 *     This file contains definition and prototype for UART configuration.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#ifndef SRC_CORE_UART_H_
#define SRC_CORE_UART_H_

extern "C" void UART_IRQHandler(void);
namespace UART {

#define BUFSIZE         64
#define AUTOBAUD_ENABLE 0
#define FDR_CALIBRATION 0
#define RS485_ENABLED   0
#define TX_INTERRUPT    0
#define MODEM_TEST      0

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

void Init(uint32_t Baudrate);
void Send(uint8_t *BufferPtr, uint32_t Length);
void SetIRQHandler(void(*handler)(void));
void DefaultIRQHandler(void);

}  // namespace UART

#endif  // SRC_CORE_UART_H_
