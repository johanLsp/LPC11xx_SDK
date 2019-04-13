#MIT License
#
#Copyright (c) 2017 catch22eu
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
##LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.


DEVICE      = cortex-m0
# PORT depends on OS:
# For Linux:
PORT        = /dev/ttyUSB0
BAUD        = 115200
#ISPCLK      = 14746
ISPCLK = 12000
#BAUD       = 9600
FILENAME    = main
GCC         = arm-none-eabi-g++
LD			= arm-none-eabi-ld
OBJCP		= arm-none-eabi-objcopy
#OPT			= -Wall -Os -std=c99 -nostartfiles -mcpu=$(DEVICE) -mthumb -Wl,-Map,output.map
#OPT			= -Wall -Os -std=c99 -specs=nosys.specs -DUSE_OLD_STYLE_DATA_BSS_INIT -mcpu=$(DEVICE) -mthumb -Wl,-Map,output.map
OPT			= -Wall -Os -std=c++11 -fno-exceptions -specs=nosys.specs -DUSE_OLD_STYLE_DATA_BSS_INIT -mcpu=$(DEVICE) -mthumb -Wl,-Map,output.map
DEF			= -DCORE_M0 -DENABLE_UNTESTED_CODE


SRC_DIR=src
BUILD_DIR=build
CORE_DIR=$(SRC_DIR)/core

CORE = timer \
       gpio \
	   uart \
	   ssp \
	   system_LPC11xx \
	   core_cm0 \
	   cr_startup_lpc11





DIRS = .
SRC_DIRS = $(foreach dir, $(DIRS), $(addprefix $(SRC_DIR)/, $(dir)))
TARGET_DIRS = $(foreach dir, $(DIRS), $(addprefix $(BUILD_DIR)/, $(dir)))

# Generate the GCC includes parameters by adding -I before each source folder
INCLUDES = $(foreach dir, $(SRC_DIRS), $(addprefix -I, $(dir)))
INCLUDES += $(addprefix -I, $(CORE_DIR))
# Add this list to VPATH, the place make will look for the source files
VPATH = $(SRC_DIRS) $(CORE_DIR)

SRC=$(wildcard src/*.cpp)
CORE_SRC=$(addprefix  $(CORE_DIR)/, $(addsuffix .c, $(CORE)))

CORE_OBJS := $(subst $(SRC_DIR),$(BUILD_DIR),$(CORE_SRC:.c=.o))
OBJS := $(subst $(SRC_DIR),$(BUILD_DIR),$(SRC:.cpp=.o))

ELF=$(BUILD_DIR)/$(FILENAME).elf
BIN=$(BUILD_DIR)/$(FILENAME).bin

# Define linker script file here
LINKER_SCRIPT = lpc1114.ld

all: clean build upload

$(BUILD_DIR)/%.o: %.c
	$(GCC) -c $(OPT) $(INCLUDES) $(DEF) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(GCC) -c $(OPT) $(INCLUDES) $(DEF) $< -o $@
	
$(BUILD_DIR)/%.elf: $(CORE_OBJS) $(OBJS)
#	$(GCC) $(OPT) $(LIBS) -nostartfiles $(OBJS) -o $@
#	$(GCC) $(OPT) $(LIBS) -nostartfiles -T$(LINKER_SCRIPT) $(OBJS) -o $@
	$(GCC) $(OPT) $(LIBS) -T$(LINKER_SCRIPT) $(CORE_OBJS) $(OBJS) -o $@

%bin: %elf
	$(OBJCP) -O binary -S $< $@

build: $(CORE_OBJS) $(OBJS) $(ELF) $(BIN)

upload: $(BIN)
	lpc21isp -bin $(BIN) $(PORT) $(BAUD) $(ISPCLK)

clean:
	rm -f $(CORE_OBJS) $(OBJS) $(ELF) $(BIN)
