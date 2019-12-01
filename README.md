# LPC11xx SDK
High level and easy to use abstraction layer for NXP LPC11xx microcontrollers.

It provides a straightforward way to build and upload your application to a microcontroller
and offers a set of ready to use core components and more advanced modules.
See the sections below for a short description of components included.


## Getting Started
Plug your microcontroller, run make and let the SDK take care of everything.

```
cd examples/blinky
make
```
This will build the blinky example and try to upload it using the /dev/ttyUSB0 serial interface.
The example is 50 lines long, Makefile included and can be easily extended using other core components and modules.

## Core Components
The core components are the abstraction layer over the LPC peripherals. The following core components are available:
- adc
- gpio
- uart
- ssp
- i2c
- timer

Core components are excluded by default to save space. To include a component, simply add it to the CORE variable in the project Makefile:
```
CORE = gpio timer uart
```

Here's an example using the gpio component:
```
#include "core/gpio.h"
  
int main(int argc, char** argv) {
  GPIO::Init(GPIO::PORT0);
  GPIO output({GPIO::PORT0, 5}, GPIO::OUTPUT);

  output.Off();
  output.Toggle();
}
```

## Modules
The modules are external components that are interfaced with the microcontroller. Modules build on top of the core components. Currently available modules are:
- dac
- display
- encoder
- knob
- midi
- segment


Modules are excluded by default to save space. To include a module, simply add it to the MODULES variable in the project Makefile:
```
MODULES = encoder dac
```
