// Copyright 2019 Johan Lasperas
#include "uart.h"

namespace UART {
volatile uint32_t RegVal;
volatile uint32_t Status;
volatile uint8_t  TxEmpty = 1;
volatile uint8_t  Buffer[BUFSIZE];
volatile uint32_t Count = 0;
Handler handler_;
}

void UART_IRQHandler(void) {
  UART::handler_();
}

void UART::SetIRQHandler(Handler handler) {
  handler_ = handler;
}

void UART::DefaultIRQHandler() {
  uint8_t IIRValue = LPC_UART->IIR;

  // skip pending bit in IIR
  IIRValue >>= 1;
  // check bit 1~3, interrupt identification
  IIRValue &= 0x07;
  // Receive Line Status
  if (IIRValue == IIR::RLS) {
    uint8_t LSRValue = LPC_UART->LSR;
    // Receive Line Status
    if (LSRValue & (LSR::OE | LSR::PE | LSR::FE | LSR::RXFE | LSR::BI)) {
      // There are errors or break interrupt
      // Read LSR will clear the interrupt
      Status = LSRValue;
      // Dummy read on RX to clear interrupt, then bail out
      RegVal = LPC_UART->RBR;
      return;
    }
    // Receive Data Ready
    if (LSRValue & LSR::RDR) {
      // If no error on RLS, normal ready, save into the data buffer.
      // Note: read RBR will clear the interrupt
      Buffer[Count++] = LPC_UART->RBR;
      if (Count == BUFSIZE) {
        // buffer overflow
        Count = 0;
      }
    }
  // Receive Data Available
  } else if (IIRValue == IIR::RDA) {
    Buffer[Count++] = LPC_UART->RBR;
    if (Count == BUFSIZE) {
      // buffer overflow
      Count = 0;
    }
  // Character timeout indicator
  } else if (IIRValue == IIR::CTI) {
    // Bit 9 as the CTI error
    Status |= 0x100;
  // THRE, transmit holding register empty
  } else if (IIRValue == IIR::THRE) {
    // Check status in the LSR to see if valid data in U0THR or not
    uint8_t LSRValue = LPC_UART->LSR;
    if (LSRValue & LSR::THRE) {
      TxEmpty = 1;
    } else {
      TxEmpty = 0;
    }
  }
  return;
}

void UART::Init(const Config& config) {
  uint32_t Fdiv;
  TxEmpty = 1;
  Count = 0;

  SetIRQHandler(DefaultIRQHandler);

  NVIC_DisableIRQ(UART_IRQn);
  // Enable IOCON clock
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_IOCON;
  // UART I/O config
  // UART RXD
  LPC_IOCON->PIO1_6 &= ~0x07;
  LPC_IOCON->PIO1_6 |= 0x01;
  // UART TXD
  LPC_IOCON->PIO1_7 &= ~0x07;
  LPC_IOCON->PIO1_7 |= 0x01;

  // Enable UART clock
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_UART;
  // Divided by 1
  LPC_SYSCON->UARTCLKDIV = 0x1;

  // 8 bits, no Parity, 1 Stop bit
  LPC_UART->LCR = config.bits;
  if (config.parity != NONE) {
    LPC_UART->LCR |= LCR::PE;
    LPC_UART->LCR |= config.parity << 4;
  }
  LPC_UART->LCR |= config.stopbits << 2;
  LPC_UART->LCR |= LCR::DLAB;

  Fdiv = ((SystemCoreClock/LPC_SYSCON->UARTCLKDIV)/16)/config.baudrate;
  LPC_UART->DLM = Fdiv / 256;
  LPC_UART->DLL = Fdiv % 256;
  LPC_UART->FDR = 0x10;
  // DLAB = 0
  LPC_UART->LCR &= ~LCR::DLAB;
  // Enable and reset TX and RX FIFO.
  LPC_UART->FCR = FCR::FIFOEN | FCR::RXFIFORES | FCR::TXFIFORES;

  // Read to clear the line status.
  RegVal = LPC_UART->LSR;

  // Ensure a clean start, no data in either TX or RX FIFO.
  while (( LPC_UART->LSR & (LSR::THRE|LSR::TEMT)) != (LSR::THRE|LSR::TEMT) )
    continue;
  while ( LPC_UART->LSR & LSR::RDR ) {
    // Dump data from RX FIFO
    RegVal = LPC_UART->RBR;
  }
  // Enable the UART Interrupt
  NVIC_EnableIRQ(UART_IRQn);
  // Enable UART interrupt
  LPC_UART->IER = IER::RBR | IER::RLS;
  return;
}

void UART::Send(uint8_t *BufferPtr, uint32_t Length) {
  while ( Length != 0 ) {
    while ( !(LPC_UART->LSR & LSR::THRE) ) continue;
    LPC_UART->THR = *BufferPtr;
    BufferPtr++;
    Length--;
  }
  return;
}
