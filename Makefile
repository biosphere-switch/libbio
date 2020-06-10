
ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SOURCE_DIR := $(ROOT_DIR)/libbio/source
INCLUDE_DIRS := $(ROOT_DIR)/libbio/include
OUTPUT_DIR := $(ROOT_DIR)/output/lib
BUILD_DIR := $(ROOT_DIR)/build

CPP_SOURCES := $(shell find $(SOURCE_DIR) -type f \( -iname \*.cpp -o -iname \*.cxx -o -iname \*.cc \))
C_SOURCES := $(shell find $(SOURCE_DIR) -type f -name "*.c")
S_SOURCES := $(shell find $(SOURCE_DIR) -type f \( -iname \*.s -o -iname \*.S \))

C_FLAGS := -g -fPIC -O2 -fno-builtin-memset -fno-builtin-memcpy -ffreestanding -march=armv8-a+crc+crypto -mtune=cortex-a57 -target aarch64-none-elf -nostdlib -nostdlibinc
CXX_FLAGS := $(C_FLAGS) -std=gnu++20 -fno-exceptions -nodefaultlibs -nostdinc++
AS_FLAGS := -arch=aarch64 -triple aarch64-none-elf
AR_FLAGS := rcs

include $(BIOSPHERE_ROOT)/utils/mk/compile-base.mk

TARGET_NAME := $(notdir $(ROOT_DIR))

# Module

export MODULE := $(ROOT_DIR)
export MODULE_VER := 0.1
export MODULE_OUTPUT_DIR := output

.PHONY: build install clean
.DEFAULT_GOAL: build

build: $(TARGET_NAME).a
	@rm -rf $(ROOT_DIR)/output/include
	@cp -r $(ROOT_DIR)/libbio/include/ $(ROOT_DIR)/output/
	@cp -r $(ROOT_DIR)/utils/ $(ROOT_DIR)/output/

install:
	@$(MAKE) -f $(BIOSPHERE_ROOT)/utils/mk/install-built-module.mk

clean: compile_clean
	@rm -rf $(ROOT_DIR)/output