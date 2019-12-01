// Copyright 2019 Johan Lasperas
#include "core/gpio.h"
#include "core/uart.h"
#include "core/timer.h"

/// This is a straighforward example using:
/// - GPIO: Toggle a pin on and off.
/// - Timer: Add time delays
/// - UART: Send messages via the serial interface
int main() {
  SystemInit();
  // Set PIO1_3 to output.
  GPIO led({GPIO::PORT1, 3}, GPIO::OUTPUT);
  led.On();

  // Enable Timer 32 - 1.
  Timer::Init(Timer::TIMER32_1, TIME_INTERVAL);
  Timer::Enable(Timer::TIMER32_1);

  // Configure the UART.
  UART::Config config;
  config.baudrate = 9600;
  config.bits = UART::Bits::WLS8;
  config.parity = UART::Parity::NONE;
  config.stopbits = UART::StopBits::SB1;
  UART::Init(config);

  while (true) {
    led.Toggle();
    UART::Send("I'm alive!\n");
    Timer::DelayMs(Timer::TIMER32_1, 200);
  }
  return 0;
}
