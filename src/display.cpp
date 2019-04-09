#include "display.hpp"

// Declare arrays as const to flash them to memory

// Index 10 : ' '
const uint8_t charset[64] = {
// Index 0-9 : '0' -> '9'
0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B,
// Index 10-35 : 'A' -> 'Z'
0x77, 0x7F, 0x4E, 0x7E, 0x4F, 0x47, 0x5E, 0x37, 0x30, 0x3C,
0x37, 0x0E, 0x55, 0x15, 0x7E, 0x67, 0x73, 0x77, 0x5B, 0x46,
0x3E, 0x27, 0x3F, 0x25, 0x3B, 0x6D,
// Index 36-61 : 'a' -> 'z'
0x7D, 0x1F, 0x0D, 0x3D, 0x6f, 0x47, 0x7B, 0x17, 0x10, 0x38,
0x17, 0x06, 0x55, 0x15, 0x1D, 0x67, 0x73, 0x05, 0x5B, 0x0F,
0x1C, 0x23, 0x2B, 0x25, 0x33, 0x6D,
// Index 62-63 : ' ', '-'
0x00, 0x01};

const uint8_t digit_port[4] = {0, 0, 0, 1};
const uint8_t digit_pin[4] = {10, 8, 9, 0};

const uint8_t segment_port[8] = {1, 0, 0, 1, 1, 1, 0, 1};
const uint8_t segment_pin[8] = {3, 11, 6, 4, 2, 1, 5, 5};

volatile char msg[4];
uint8_t currentDigit;

uint32_t autoShutdown;
uint32_t currentShutdown;

void Display::Print(const char* m) {
  for (int i = 0; i < 4; i++) {
    char c = m[i];
    if (c < 16) {
      if (c > 9) {
        msg[i] = c - 10 + 36;
      } else {
        msg[i] = c;
      }
    } else if (c == '-') {
      msg[i] = 63;
    } else if (c == ' ' || c < '0') {
      msg[i] = 62;
    } else if (c <= '9') {
      msg[i] = c - '0';
    } else if (c <= 'Z') {
      msg[i] = c - 'A' + '9' - '0' + 1;
    } else if (c <= 'z') {
      msg[i] = c - 'a' + 'Z' - 'A' + 1 + '9' - '0' + 1;
    } else {
      msg[i] = ' ';
    }
  }

  if (autoShutdown != 0 && currentShutdown == 0) {
    enable_timer16(TIMER16_1);
  }
}

void Display::Clear() {
  for (uint8_t i = 0; i < 4; i++) {
    GPIO::SetValue(digit_port[i], digit_pin[i], 0);
  }
  for (uint8_t i = 0; i < 8; i++) {
    GPIO::SetValue(segment_port[i], segment_pin[i], 1);
  }
}

void Display::Refresh() {
    Clear();
    uint8_t value = charset[msg[currentDigit]];
    for (uint8_t j = 0; j < 8; j++) {
      if (value & (0x01 << (7-j))) {
        GPIO::SetValue(segment_port[j], segment_pin[j], 0);
      }
    }
    GPIO::SetValue(digit_port[currentDigit], digit_pin[currentDigit], 1);

    if (currentDigit++ >= 4) {
      currentDigit = 0;
    }

    if (autoShutdown != 0 && currentShutdown++ > autoShutdown) {
      currentShutdown = 0;
      disable_timer16(TIMER16_1);
      Clear();
    }
}

void Display::AutoShutdown(uint32_t shutdown) {
  autoShutdown = shutdown;
  currentShutdown = 0;
}

void Display::Init() {
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16); // enable IOCON
  /* Enable AHB clock to the GPIO domain. */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);

  LPC_IOCON->R_PIO1_0  &= ~0x07;
  LPC_IOCON->R_PIO1_0  |= 0x01;
  LPC_IOCON->R_PIO1_1  &= ~0x07;
  LPC_IOCON->R_PIO1_1  |= 0x01;
  LPC_IOCON->R_PIO1_2  &= ~0x07;
  LPC_IOCON->R_PIO1_2  |= 0x01;
  LPC_IOCON->SWDIO_PIO1_3  &= ~0x07;
  LPC_IOCON->SWDIO_PIO1_3  |= 0x01;

  LPC_IOCON->PIO1_4  &= ~0x07;
  LPC_IOCON->PIO1_5  &= ~0x07;

  LPC_IOCON->PIO0_8  &= ~0x07;
  LPC_IOCON->PIO0_9  &= ~0x07;
  LPC_IOCON->SWCLK_PIO0_10 &= ~0x07;
  LPC_IOCON->SWCLK_PIO0_10 |= 0x01;
  LPC_IOCON->R_PIO0_11  &= ~0x07;
  LPC_IOCON->R_PIO0_11  |= 0x01;
  LPC_IOCON->PIO0_5  &= ~0x07;
  LPC_IOCON->PIO0_6  &= ~0x07;

  for (uint8_t i = 0; i < 4; i++) {
    GPIO::SetDirection(digit_port[i], digit_pin[i], 1);
    GPIO::SetValue(digit_port[i], digit_pin[i], 1);
  }
  for (uint8_t i = 0; i < 8; i++) {
    GPIO::SetDirection(segment_port[i], segment_pin[i], 1);
    GPIO::SetValue(segment_port[i], segment_pin[i], 1);
  }

  currentDigit = 0;
  Print("    ");
    // Enable Timer 16 - 1
  //init_timer16(TIMER16_1, TIME_INTERVAL);
  init_timer16(TIMER16_1, 0x3FFF);
  enable_timer16(TIMER16_1);
  set_timer16IRQHandler(TIMER16_1, Refresh, true);
  autoShutdown = 0;
  currentShutdown = 0;
}
