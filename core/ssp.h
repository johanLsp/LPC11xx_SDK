// Copyright 2019 Johan Lasperas
#ifndef SRC_CORE_SSP_H_
#define SRC_CORE_SSP_H_

#include "LPC11xx.h"

extern "C" void SSP0_IRQHandler();
extern "C" void SSP1_IRQHandler();

namespace SSP {
enum SSP {SSP0 = 0, SSP1};
typedef void (*Handler)(SSP);

struct SSP_Config {
  bool slave;
  bool loopback;
  bool use_CS;
  bool use_MISO;
  bool use_MOSI;
  uint8_t frame_size;
};

void SetIRQHandler(SSP ssp, Handler handler);
void DefaultIRQHandler(SSP ssp);

void Init(SSP ssp, SSP_Config config);
void IOConfig(SSP ssp, SSP_Config config);
void SetFrameSize(SSP ssp, uint8_t size);

void Send(SSP ssp, uint8_t *buffer, uint32_t size);
void Receive(SSP ssp, uint8_t *buffer, uint32_t size);


}  // namespace SSP

/* SPI read and write buffer size */
#define SSP_BUFSIZE     16
#define FIFOSIZE        8

#define DELAY_COUNT     10
#define MAX_TIMEOUT     0xFF

/* Port0.2 is the SSP select pin */
#define SSP0_SEL        (0x1<<2)
    
/* SSP Status register */
#define SSPSR_TFE       (0x1<<0)
#define SSPSR_TNF       (0x1<<1) 
#define SSPSR_RNE       (0x1<<2)
#define SSPSR_RFF       (0x1<<3) 
#define SSPSR_BSY       (0x1<<4)

/* SSP CR0 register */
#define SSPCR0_DSS      (0x1<<0)
#define SSPCR0_FRF      (0x1<<4)
#define SSPCR0_SPO      (0x1<<6)
#define SSPCR0_SPH      (0x1<<7)
#define SSPCR0_SCR      (0x1<<8)

/* SSP CR1 register */
#define SSPCR1_LBM      (0x1<<0)
#define SSPCR1_SSE      (0x1<<1)
#define SSPCR1_MS       (0x1<<2)
#define SSPCR1_SOD      (0x1<<3)

/* SSP Interrupt Mask Set/Clear register */
#define SSPIMSC_RORIM   (0x1<<0)
#define SSPIMSC_RTIM    (0x1<<1)
#define SSPIMSC_RXIM    (0x1<<2)
#define SSPIMSC_TXIM    (0x1<<3)

/* SSP0 Interrupt Status register */
#define SSPRIS_RORRIS   (0x1<<0)
#define SSPRIS_RTRIS    (0x1<<1)
#define SSPRIS_RXRIS    (0x1<<2)
#define SSPRIS_TXRIS    (0x1<<3)

/* SSP0 Masked Interrupt register */
#define SSPMIS_RORMIS   (0x1<<0)
#define SSPMIS_RTMIS    (0x1<<1)
#define SSPMIS_RXMIS    (0x1<<2)
#define SSPMIS_TXMIS    (0x1<<3)

/* SSP0 Interrupt clear register */
#define SSPICR_RORIC    (0x1<<0)
#define SSPICR_RTIC     (0x1<<1)

/* RDSR status bit definition */
#define RDSR_RDY        0x01
#define RDSR_WEN        0x02

/* If RX_INTERRUPT is enabled, the SSP RX will be handled in the ISR
SSPReceive() will not be needed. */

#endif  // SRC_CORE_SSP_H_
