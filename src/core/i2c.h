// Copyright 2019 Johan Lasperas
#ifndef SRC_CORE_I2C_H_
#define SRC_CORE_I2C_H_

#include "LPC11xx.h"

extern "C" void I2C_IRQHandler();

namespace I2C {
static const int kBufferSize = 4 * 128 + 1;
typedef void (*Handler)();


enum Control {
  ACK = 0x1 << 2,
  INTERRUPT = 0x1 << 3,
  STOP = 0x1 << 4,
  START = 0x1 << 5,
  ENABLE = 0x1 << 6
};

enum Direction {
  READ = 0x1,
  WRITE = 0x0
};

enum class Status {
  IDLE,
  SUCCESS,
  ERROR,
  WRITING
};

void SetIRQHandler(Handler handler);
void DefaultIRQHandler();

void Init();
bool Write(uint8_t address, uint8_t reg, const uint8_t* buffer,
           uint32_t length);
bool Read(uint8_t address, uint8_t reg, uint8_t* buffer, uint32_t length);
uint8_t Scan();

void SetControl(Control control);
void ClearControl(Control control);

}  // namespace I2C

#define I2CMASTER           0x01
#define I2CSLAVE            0x02

#define PCF8594_ADDR        0xA0
#define READ_WRITE          0x01

#define RD_BIT              0x01

#define I2C_IDLE              0
#define I2C_STARTED           1
#define I2C_RESTARTED         2
#define I2C_REPEATED_START    3
#define DATA_ACK              4
#define DATA_NACK             5
#define I2C_BUSY              6
#define I2C_NO_DATA           7
#define I2C_NACK_ON_ADDRESS   8
#define I2C_NACK_ON_DATA      9
#define I2C_ARBITRATION_LOST  10
#define I2C_TIME_OUT          11
#define I2C_OK                12

#define I2DAT_I2C           0x00000000  /* I2C Data Reg */
#define I2ADR_I2C           0x00000000  /* I2C Slave Address Reg */
#define I2SCLH_SCLH         0x00000180  /* I2C SCL Duty Cycle High Reg */
#define I2SCLL_SCLL         0x00000180  /* I2C SCL Duty Cycle Low Reg */
#define I2SCLH_HS_SCLH		0x00000015  /* Fast Plus I2C SCL Duty Cycle High Reg */
#define I2SCLL_HS_SCLL		0x00000015  /* Fast Plus I2C SCL Duty Cycle Low Reg */

extern uint32_t I2CInit( uint32_t I2cMode );
extern uint32_t I2CStart( void );
extern uint32_t I2CStop( void );
extern uint32_t I2CEngine( void );

#endif  // SRC_CORE_I2C_H_
