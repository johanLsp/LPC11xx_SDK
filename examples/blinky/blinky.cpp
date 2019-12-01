// Copyright 2019 Johan Lasperas
#include "core/gpio.h"
#include "core/uart.h"
#include "core/timer.h"

/// This is a straighforward example using:
/// - GPIO: Toggle a pin on and off.
/// - Timer: Add time delays
/// - UART: Send messages via the serial interface
int main(int argc, char** argv) {
  // Set PIO0_5 to output.
  GPIO::Init(GPIO::PORT0);
  GPIO led({GPIO::PORT0, 5}, GPIO::OUTPUT);

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
    UART::Send("I'm alive!");
    Timer::DelayMs(Timer::TIMER32_1, 200);
  }
  return 0;
}
