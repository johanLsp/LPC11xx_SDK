// Copyright 2019 Johan Lasperas
#include "i2c.h"

namespace I2C {
Handler handler_;

uint8_t slave_address_;
uint8_t register_;
uint8_t* read_buffer_;
const uint8_t* write_buffer_;

volatile uint32_t length_;
volatile uint32_t index_;
bool read_;

bool debug_ = false;
bool found_ = false;
bool done_ = false;

volatile Status status_; 
volatile uint8_t states_[16] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile uint8_t state_index_ = 0;

volatile uint32_t I2CMasterState = I2C_IDLE;
volatile uint32_t I2CSlaveState = I2C_IDLE;
volatile uint32_t timeout = 0;
volatile uint32_t I2CCount = 0;
volatile uint32_t I2CReadLength;
volatile uint32_t I2CWriteLength;

volatile uint32_t RdIndex = 0;
volatile uint32_t WrIndex = 0;
}  // namespace I2C

void I2C_IRQHandler() {
  I2C::handler_();
}

void I2C::SetIRQHandler(Handler handler) {
  handler_ = handler;
}

// Each event on the I2C bus generates an interrupt.
// At the moment, only Master mode is implemented, which
// uses the following states:
//  - 0x08 : Start bit sent
//  - 0x10 : Repeated start bit sent
//  - 0x18 : Address sent (Write), ACK received
//  - 0x20 : Address sent (Write), NOT ACK received
//  - 0x28 : Data sent, ACK received
//  - 0x30 : Data sent, NOT ACK received
//  - 0x38 : Arbitration lost during ACK
//  - 0x40 : Address sent (Read), ACK received
//  - 0x48 : Address sent (Read), NOT ACK received
void I2C::DefaultIRQHandler() {
  uint8_t StatValue = LPC_I2C->STAT;
  if (state_index_ < 16) {
    states_[state_index_++] = StatValue;
  }
  switch (StatValue) {
    // Bus error
    case 0x00:
      status_ = Status::ERROR;
      SetControl(ACK);
      SetControl(STOP);
      break;
    // Start bit sent
    case 0x08:
    // Repeated start sent
    case 0x10:
      // Write slave address and read/write bit
      LPC_I2C->DAT = slave_address_ | (read_ ? READ : WRITE);
      break;
    // Address sent (Write), ACK received
    case 0x18:
      found_ = true;
      done_ = true;
      // Load data
      ClearControl(START);
      LPC_I2C->DAT = register_;
      break;
    // Address sent (Write), NOT ACK received
    case 0x20:
      done_ = true;
      // Repeat start
      ClearControl(START);
      LPC_I2C->DAT = register_;
      break;
    // Data sent, ACK received
    case 0x28:
      found_ = true;
      done_ = true;
      if (index_ < length_) {
        LPC_I2C->DAT = write_buffer_[index_++];
      } else {
        // Send stop
        status_ = Status::SUCCESS;
        SetControl(STOP);
      }
      break;
    // Data sent, NOT ACK received
    case 0x30:
      done_ = true;
      if (index_ < length_) {
        LPC_I2C->DAT = write_buffer_[index_++];
      } else {
        status_ = Status::SUCCESS;
        // Send stop
        SetControl(STOP);
      }
      break;
    // Arbitration lost
    case 0x38:
      // Restart when possible
      SetControl(START);
      break;
    // Address sent (Read), ACK received
    case 0x40:
      done_ = true;
      found_ = true;
      // Acknowledge
      SetControl(ACK);
      break;
    // Address sent (Read), NOT ACK received
    case 0x48:
      done_ = true;
      break;
    // Data received, ACK received
    case 0x50:
      // Read data
      read_buffer_[index_++] = LPC_I2C->DAT;
      // Continue
      SetControl(ACK);
      break;
    // Address sent (Read), NOT ACK received
    case 0x58:
      // Read data
      read_buffer_[index_] = LPC_I2C->DAT;
      // Stop
      SetControl(STOP);
      break;
  }

  // Reset the interrupt
  ClearControl(INTERRUPT);
}

uint8_t I2C::Scan() {
  found_ = false;
  while (!found_) {
    for (uint8_t address = 0; address < 256; address+=2) {
      done_ = false;
      uint8_t reg = 0;
      uint8_t buffer = 0;
      Write(address, reg, &buffer, 1);
      while (!done_) continue;
      if (found_) return address;
    }
  }
  return 0;
}

void I2C::Init() {
  states_[0] = 1;
  states_[1] = 2;
  states_[2] = 3;
  states_[3] = 4;
  state_index_ = 0;
  // Reset I2C
  LPC_SYSCON->PRESETCTRL |= (0x1<<1);
  // Enable the clock for the I2C domain
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;
  LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_I2C;

  // Set PIO0_4 pin function to SCL
  LPC_IOCON->PIO0_4 &= ~0x07;
  LPC_IOCON->PIO0_4  |= 0x01;
  // Set I2C mode to Fast-mode I2C
  // 0 = Standard mode/ Fast-mode I2C.
  // 1 = Standard I/O functionality
  // 2 = Fast-mode Plus I2C
  LPC_IOCON->PIO0_4 &= ~(0x3 << 8);
  LPC_IOCON->PIO0_4 |= 0x2 << 8;
  // Set PIO0_5 pin function to SDA
  LPC_IOCON->PIO0_5 &= ~0x07;
  LPC_IOCON->PIO0_5  |= 0x01;
  // Set I2C mode to Fast-mode I2C
  // 0 = Standard mode/ Fast-mode I2C.
  // 1 = Standard I/O functionality
  // 2 = Fast-mode Plus I2C
  LPC_IOCON->PIO0_5 &= ~(0x3 << 8);
  LPC_IOCON->PIO0_5 |= 0x2 << 8;

  LPC_I2C->SCLL   = 0x00000012;
  LPC_I2C->SCLH   = 0x00000012;
  // Clear control bits
  ClearControl(START);
  ClearControl(STOP);
  ClearControl(INTERRUPT);
  ClearControl(ENABLE);

  // Clear ack: device cannot enter slave mode
  ClearControl(ACK);
  status_ = Status::IDLE;
  SetIRQHandler(DefaultIRQHandler);
  // Enable the I2C Interrupt
  NVIC_EnableIRQ(I2C_IRQn);
  // Enable I2C
  SetControl(ENABLE);
}

bool I2C::Write(uint8_t address, uint8_t reg, const uint8_t* buffer,
                uint32_t length) {
  slave_address_ = address;
  register_ = reg;
  write_buffer_ = buffer;
  length_ = length;
  index_ = 0;
  read_ = 0;
  status_ = Status::WRITING;
  SetControl(START);

  uint32_t timeout = 0;
  uint32_t max_timeout = 0x00FFFFFF;
  while (status_ == Status::WRITING) {
    if (timeout >= max_timeout) {
      return false;
    }
    timeout++;
  }
  return status_ == Status::SUCCESS;
}

void I2C::Read(uint8_t address, uint8_t reg, uint8_t* buffer, uint32_t length) {
  slave_address_ = address;
  register_ = reg;
  read_buffer_ = buffer;
  length_ = length;
  index_ = 0;
  read_ = 1;
  SetControl(START);
}

void I2C::SetControl(Control control) {
  LPC_I2C->CONSET = control;
}

void I2C::ClearControl(Control control) {
  LPC_I2C->CONCLR = control;
}
