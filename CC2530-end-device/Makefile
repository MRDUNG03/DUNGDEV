# Z-Stack 3.0.2 Makefile for SensorApp
# This should be integrated into Z-Stack build system

# Z-Stack paths - Set ZSTACK_PATH environment variable or modify here
ifndef ZSTACK_PATH
ZSTACK_PATH = D:/Z-Stack_3.0.2
endif

# Check if Z-Stack path exists
ifeq ($(wildcard $(ZSTACK_PATH)),)
$(error Z-Stack path not found: $(ZSTACK_PATH). Please set ZSTACK_PATH environment variable)
endif

COMPONENTS_PATH = $(ZSTACK_PATH)/Components
HAL_PATH = $(COMPONENTS_PATH)/hal
OSAL_PATH = $(COMPONENTS_PATH)/osal
MAC_PATH = $(COMPONENTS_PATH)/mac
NWK_PATH = $(COMPONENTS_PATH)/nwk
APS_PATH = $(COMPONENTS_PATH)/aps
ZDO_PATH = $(COMPONENTS_PATH)/zdo
ZCL_PATH = $(COMPONENTS_PATH)/zcl

# Compiler settings
CC = sdcc
CFLAGS = -mmcs51 --model-large --std-c99 -DZSTACK_DEVICE_BUILD -DENDDEVICE_ONLY
CFLAGS += -I. -Iz_stack_config.h
CFLAGS += -I$(HAL_PATH)/include -I$(HAL_PATH)/target/CC2530EB
CFLAGS += -I$(OSAL_PATH)/include -I$(COMPONENTS_PATH)/zmac
CFLAGS += -I$(MAC_PATH)/include -I$(NWK_PATH)/include
CFLAGS += -I$(APS_PATH)/include -I$(ZDO_PATH)/include -I$(ZCL_PATH)/include
CFLAGS += -I$(ZSTACK_PATH)/Projects/zstack/ZMain/TI2530EB
CFLAGS += -I$(ZSTACK_PATH)/Projects/zstack/ZMain

# Linker settings
LDFLAGS = --xram-loc 0x0000 --xram-size 0x1F00 --code-loc 0x8000 --code-size 0x7800

# Application files
APP_SOURCES = main.c sht30.c ldr.c GP2Y.c
APP_OBJECTS = $(APP_SOURCES:.c=.rel)
TARGET = SensorApp

# Z-Stack library paths (adjust based on actual Z-Stack installation)
ZSTACK_LIB = $(ZSTACK_PATH)/Projects/zstack/ZMain/TI2530EB/ZStack.lib
ifeq ($(wildcard $(ZSTACK_LIB)),)
ZSTACK_LIB = $(ZSTACK_PATH)/bin/ZStack.lib
endif

# Default target
all: $(TARGET).hex

# Compile application files
%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link with Z-Stack
$(TARGET).hex: $(APP_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(APP_OBJECTS) $(ZSTACK_LIB) -o $(TARGET).ihx
	packihx $(TARGET).ihx > $(TARGET).hex

# Clean build files
clean:
	rm -f *.rel *.ihx *.hex *.lst *.map *.mem *.lk *.rst *.sym *.asm

# Flash to device
flash: $(TARGET).hex
	cc-tool -e -w $(TARGET).hex

.PHONY: all clean flash
