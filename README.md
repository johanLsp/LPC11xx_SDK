# LPC11xx SDK
High level and easy to use abstraction layer for NXP LPC11xx microcontrollers.

It provides a straightforward way to build and upload your application to a microcontroller
and offers a set of ready to use core components and more advanced modules.
See the sections below for a short description of components included.

## Requirements
This SDK is initially intended to work on linux using the arm-none-eabi toolchain, but you can easily adapt the toolchain used in sdk.mk.

On Ubuntu, you can install the arm toolchain using:
```
sudo apt install libc6-armel-cross libc6-dev-armel-cross binutils-arm-linux-gnueabi libncurses5-dev
```

Uploading the image to the microcontroller is done using lpc21isp. You can also change that easily in sdk.mk.
To get it on Ubuntu, run:
```
sudo apt install lpc21isp
```

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
  
int main() {
  SystemInit();
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

## Going further
The blinky example was fun but you want to build your own project?
Simply copy over the blinky Makefile to your project root directory and update it with your configuration.

Let say your project is to have a 128x32 OLED display attached to your LPC1114 microcontroller and be able to send text through the UART that will appear on the display.

You'll need the *display* module and the *uart* and *i2c* core component as the communication with the display uses I2C.

Let's assume your project tree structure is the following:

```
project
|__ build
|
|__ LPC11xx_SDK
|
|__ src
|    |__ other.cpp
|    |__ other.hpp
|    |__ main.cpp
|
|__ Makefile
```

Your Makefile will look as follows:
```
# Add the core component that are required.
# Available core components:
# adc clkconfig gpio i2c ssp timer uart
CORE = uart i2c

# Add modules from the modules folder that you want to use.
# Available modules:
# dac display encoder knob midi segment
MODULES = display

# Update the following variable to your project structure.
BUILD_DIR = build
SDK_DIR = LPC11xx_SDK
SRC_DIR = src
FILENAME = main

LINKER_SCRIPT = lpc1114.ld

include $(SDK_DIR)/sdk.mk
```
