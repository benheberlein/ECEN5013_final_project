# Sources
SRCS := main.c \
        system_stm32f4xx.c \
        startup_stm32f429_439xx.s \
        stm32f4xx_it.c

# Object files
OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)

# Assembly files
ASMS := $(SRCS:.c=.s)

# Search path for source files
VPATH = src:src/int_drv:src/ext_drv:src/startup

# Include directory
INC_DIR := inc \
           inc/ext_drv \
           inc/inc_drv \
           inc/startup

# Config directory
CONF_DIR := config

# Build directory
BUILD_DIR := build

# Binary directory
BIN_DIR := bin

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME := firmware_poc

# Linker file
LINKER_FILE := $(CONF_DIR)/STM32F429ZI_FLASH.ld

