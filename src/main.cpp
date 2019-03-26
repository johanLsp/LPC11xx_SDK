// Copyright 2019 Johan Lasperas

#include "core/LPC11xx.h"
#include "core/gpio.h"
#include "core/timer32.h"

#define TIMER0 0

int main(void) {
  SystemCoreClockUpdate();

  // Enable Timer 32 - 0
  init_timer32(TIMER0, TIME_INTERVAL);
  enable_timer32(TIMER0);

  GPIOInit();
  // Set pin 5 of GPIO1 as output.
  GPIOSetDir(PORT1, 5, 1);

  // Flash LED to notify startup
  GPIOSetValue(PORT1, 5, 1);
  delay32Ms(TIMER0, 200);
  GPIOSetValue(PORT1, 5, 0);
  delay32Ms(TIMER0, 200);
  GPIOSetValue(PORT1, 5, 1);
  delay32Ms(TIMER0, 200);
  GPIOSetValue(PORT1, 5, 0);
  delay32Ms(TIMER0, 200);
  GPIOSetValue(PORT1, 5, 1);
  delay32Ms(TIMER0, 200);
  GPIOSetValue(PORT1, 5, 0);
  delay32Ms(TIMER0, 200);

  while (1) {}
}
