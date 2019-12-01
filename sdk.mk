# This Makefile is not meant to be called directly but to be
# included from a project main Makefile.
# It gathers all common build functionality and allows to quickly
# write a simple Makefile to start a project;

# The main Makefile calling it should define the following variables:
# CORE : List of core components to use
# MODULES : List of modules to use
#
# BUILD_DIR : The directory to store the build files
# SDK_DIR : The root directory of the LPC11xx SDK
# SRC_DIR : The root directory of the project sources
# FILENAME : The name of the target to build
# LINKER_SCRIPT : The linker script to use, i.e. lpc111x.ld
#
# And finally include this Makefile:
# include $(SDK_DIR)/sdk.mk

DEVICE      = cortex-m0

# Used for uploading the resulting image to the target
PORT          = /dev/ttyUSB0
BAUD          = 115200
ISPCLK        = 12000
LPC21ISP      = lpc21isp
LPC21ISP_OPTS = -control

# Build related parameters
GCC           = arm-none-eabi-g++
LD			      = arm-none-eabi-ld
OBJCP		      = arm-none-eabi-objcopy
DEF			      = -DCORE_M0 -DENABLE_UNTESTED_CODE

CCFLAGS= -Wall  -std=c++11 -DUSE_OLD_STYLE_DATA_BSS_INIT -mcpu=$(DEVICE) -mthumb
# Space optimization options
CCFLAGS+= -Os -fno-exceptions -fno-rtti -flto -ffunction-sections -specs=nosys.specs
# Generate the map output
CCFLAGS+= -Wl,-Map,$(BUILD_DIR)/output.map

LDFLAGS= -Wl,--as-needed -Wl,--gc-sections

# Output .elf and .bin files
ELF=$(BUILD_DIR)/$(FILENAME).elf
BIN=$(BUILD_DIR)/$(FILENAME).bin

SRC=$(wildcard src/*.cpp)
OBJS := $(subst $(SRC_DIR),$(BUILD_DIR),$(SRC:.cpp=.o))

# Variables related to the SDK components
CORE_DIR=$(SDK_DIR)/core
MODULES_DIR=$(SDK_DIR)/modules

# Add the system core components
CORE +=	system_LPC11xx core_cm0 cr_startup_lpc11


CORE_SRC=$(addprefix $(CORE_DIR)/, $(addsuffix .c, $(CORE)))
CORE_OBJS := $(subst $(SDK_DIR),$(BUILD_DIR),$(CORE_SRC:.c=.o))

MODULES_SRC=$(addprefix $(MODULES_DIR)/, $(addsuffix .c, $(MODULES)))
MODULES_OBJS := $(subst $(SDK_DIR),$(BUILD_DIR),$(MODULES_SRC:.c=.o))
SDK_OBJS = $(CORE_OBJS) $(MODULES_OBJS)

# Add this list to VPATH, the place make will look for the source files
VPATH = $(SRC_DIRS) $(CORE_DIR) $(MODULES_DIR)

all: clean build upload

build: $(SDK_OBJS) $(OBJS) $(ELF) $(BIN)

upload: $(BIN)
	$(LPC21ISP) $(LPC21ISP_OPTS) -bin $(BIN) $(PORT) $(BAUD) $(ISPCLK)

clean:
	rm -f $(SDK_OBJS) $(OBJS) $(ELF) $(BIN)

%bin: %elf
	$(OBJCP) -O binary -S $< $@

$(BUILD_DIR)/%.o: $(SDK_DIR)/%.cpp
	@mkdir -p $(@D)
	$(GCC) -c $(CCFLAGS) -I$(SRC_DIR) -I$(SDK_DIR) $(DEF) $< -o $@

$(BUILD_DIR)/%.o: $(SDK_DIR)/%.c
	@mkdir -p $(@D)
	$(GCC) -c $(CCFLAGS) -I$(SRC_DIR) -I$(SDK_DIR) $(DEF) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(GCC) -c $(CCFLAGS) -I$(SRC_DIR) -I$(SDK_DIR) $(DEF) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(GCC) -c $(CCFLAGS) -I$(SRC_DIR) -I$(SDK_DIR) $(DEF) $< -o $@

$(BUILD_DIR)/%.elf: $(OBJS)
	$(GCC) $(CCFLAGS) $(LIBS) -T$(SDK_DIR)/$(LINKER_SCRIPT) $(LDFLAGS) $(SDK_OBJS) $(OBJS) -o $@
