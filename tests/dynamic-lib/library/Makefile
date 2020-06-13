
ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SOURCE_DIR := $(ROOT_DIR)/source
INCLUDE_DIRS := $(SOURCE_DIR) $(ROOT_DIR)/include $(BIOSPHERE_ROOT)/modules/libbio/include/
OUTPUT_DIR := $(ROOT_DIR)/output
BUILD_DIR := $(ROOT_DIR)/build

CPP_SOURCES := $(shell find $(SOURCE_DIR) -type f \( -iname \*.cpp -o -iname \*.cxx -o -iname \*.cc \))
C_SOURCES := $(shell find $(SOURCE_DIR) -type f -name "*.c")
S_SOURCES := $(shell find $(SOURCE_DIR) -type f \( -iname \*.s -o -iname \*.S \))

C_FLAGS := -g -fPIC -O2 -fno-builtin-memset -fno-builtin-memcpy -ffreestanding -march=armv8-a+crc+crypto -mtune=cortex-a57 -target aarch64-none-elf -nostdlib -nostdlibinc
CXX_FLAGS := $(C_FLAGS) -std=gnu++20 -fno-exceptions -nodefaultlibs -nostdinc++
AS_FLAGS := -arch=aarch64 -triple aarch64-none-elf
AR_FLAGS := rcs

LD_FLAGS := -T $(BIOSPHERE_ROOT)/modules/libbio/utils/link.ld -Bsymbolic --shared --eh-frame-hdr --no-undefined -Bstatic -L $(BIOSPHERE_ROOT)/modules/libbio/lib/
LD_LIBRARIES := -lbio

include $(BIOSPHERE_ROOT)/utils/mk/compile-base.mk

TARGET_NAME := $(notdir $(ROOT_DIR))

# Generate NRO and NSO
TARGET := $(TARGET_NAME).nro $(TARGET_NAME).nso

export MODULE_DEPENDENCIES := libbio

.PHONY: dependencies build clean
.DEFAULT_GOAL: build

dependencies:
	@$(MAKE) -f $(BIOSPHERE_ROOT)/utils/mk/check-module-dependencies.mk

build: dependencies $(TARGET)

clean: compile_clean