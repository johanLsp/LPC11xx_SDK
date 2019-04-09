#include "LPC11xx.h"
#include "type.h"
#include "uart.h"
#include "gpio.h"
#include "timer32.h"

namespace UART {

volatile uint32_t RegVal;
volatile uint32_t Status;
volatile uint8_t  TxEmpty = 1;
volatile uint8_t  Buffer[BUFSIZE];
volatile uint32_t Count = 0;
volatile bool uart_handler_set = false;
void (*uart_handler)(void);

void SetIRQHandler(void(*handler)(void)) {
  uart_handler = handler;
  uart_handler_set = true;
}

void UART_IRQHandler(void) {
  if (uart_handler_set) {
    uart_handler();
  } else {
    DefaultIRQHandler();
  }
}

void DefaultIRQHandler(void) {
  uint8_t IIRValue = LPC_UART->IIR;

  // skip pending bit in IIR
  IIRValue >>= 1;
  // check bit 1~3, interrupt identification
  IIRValue &= 0x07;
  // Receive Line Status
  if (IIRValue == IIR_RLS) {
    uint8_t LSRValue = LPC_UART->LSR;
    // Receive Line Status
    if (LSRValue & (LSR_OE | LSR_PE | LSR_FE | LSR_RXFE | LSR_BI)) {
      // There are errors or break interrupt
      // Read LSR will clear the interrupt
      Status = LSRValue;
      // Dummy read on RX to clear interrupt, then bail out
      RegVal = LPC_UART->RBR;
      return;
    }
    // Receive Data Ready
    if (LSRValue & LSR_RDR) {
      // If no error on RLS, normal ready, save into the data buffer.
      // Note: read RBR will clear the interrupt
      Buffer[Count++] = LPC_UART->RBR;
      if (Count == BUFSIZE) {
        // buffer overflow
        Count = 0;
      }
    }
  // Receive Data Available
  } else if (IIRValue == IIR_RDA) {
    Buffer[Count++] = LPC_UART->RBR;
    if (Count == BUFSIZE) {
      // buffer overflow
      Count = 0;
    }
  // Character timeout indicator
  } else if (IIRValue == IIR_CTI) {
    // Bit 9 as the CTI error
    Status |= 0x100;
  // THRE, transmit holding register empty
  } else if (IIRValue == IIR_THRE) {
    // Check status in the LSR to see if valid data in U0THR or not
    uint8_t LSRValue = LPC_UART->LSR;
    if (LSRValue & LSR_THRE) {
      TxEmpty = 1;
    } else {
      TxEmpty = 0;
    }
  }
  return;
}

void Init(uint32_t baudrate) {
  uint32_t Fdiv;
  TxEmpty = 1;
  Count = 0;

  NVIC_DisableIRQ(UART_IRQn);
  // Enable IOCON clock
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);
  // UART I/O config
  // UART RXD
  LPC_IOCON->PIO1_6 &= ~0x07;
  LPC_IOCON->PIO1_6 |= 0x01;
  // UART TXD
  LPC_IOCON->PIO1_7 &= ~0x07;
  LPC_IOCON->PIO1_7 |= 0x01;

  // Enable UART clock
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
  // Divided by 1
  LPC_SYSCON->UARTCLKDIV = 0x1;

  // 8 bits, no Parity, 1 Stop bit
  LPC_UART->LCR = LCR::WLS8 | LCR::DLAB;
  Fdiv = ((SystemCoreClock/LPC_SYSCON->UARTCLKDIV)/16)/baudrate ;
  LPC_UART->DLM = Fdiv / 256;
  LPC_UART->DLL = Fdiv % 256;
  LPC_UART->FDR = 0x10;
  // DLAB = 0
  LPC_UART->LCR = LCR::WLS8;
  // Enable and reset TX and RX FIFO.
  LPC_UART->FCR = FCR::FIFOEN | FCR::RXFIFORES | FCR::TXFIFORES;

  // Read to clear the line status.
  RegVal = LPC_UART->LSR;

  // Ensure a clean start, no data in either TX or RX FIFO.
  while (( LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) ) continue;
  while ( LPC_UART->LSR & LSR_RDR ) {
    // Dump data from RX FIFO
    RegVal = LPC_UART->RBR;
  }
  // Enable the UART Interrupt
  NVIC_EnableIRQ(UART_IRQn);
  // Enable UART interrupt
  LPC_UART->IER = IER_RBR | IER_RLS;
  return;
}

void Send(uint8_t *BufferPtr, uint32_t Length) {
  while ( Length != 0 ) {
    while ( !(LPC_UART->LSR & LSR_THRE) ) continue;
    LPC_UART->THR = *BufferPtr;
    BufferPtr++;
    Length--;
  }
  return;
}

}  // namespace UART
