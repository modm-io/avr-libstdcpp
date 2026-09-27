# Copyright (c) 2026, Niklas Hauser
#
# This file is part of the modm project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Shared build rules for the examples, which target an Arduino Uno. In the
# example directory:
#
#   make                         build with avr-g++ and the newest C++ standard
#   make AVR_CXX=path/to/avr-g++ use another compiler
#   make STD=c++17               use another C++ standard
#   make sim                     run in simavr, output is printed via USART0
#   make program                 flash via avrdude (set AVRDUDE_FLAGS)

AVR_CXX ?= avr-g++
EXAMPLES := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
include $(EXAMPLES)/../avr-libstdcpp.mk

NAME := $(notdir $(CURDIR))
MCU ?= atmega328p
F_CPU ?= 16000000
STD ?= $(AVR_LIBSTDCPP_STD)
MIN_STD ?= c++17
AVRDUDE_FLAGS ?= -carduino -P/dev/ttyACM0 -b115200

BUILD := build/gcc$(AVR_LIBSTDCPP_GCC)-$(STD)
SOURCES := $(wildcard *.cpp) $(wildcard $(EXAMPLES)/common/*.cpp) \
           $(wildcard $(EXAMPLES)/../runtime/*.cpp) $(AVR_LIBSTDCPP_SOURCES)
OBJECTS := $(addprefix $(BUILD)/,$(notdir $(SOURCES:=.o)))
ELF := $(BUILD)/$(NAME).elf

CXXFLAGS := -std=$(STD) -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -I$(AVR_LIBSTDCPP_INCLUDE) -I$(EXAMPLES)/common \
            -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fno-threadsafe-statics \
            -ffunction-sections -fdata-sections
# Compound assignments to volatile registers are deprecated in C++20 only
CXXFLAGS += -Wno-volatile
# GCC 12 and 13 wrongly warn about every register access (GCC bug 105523)
ifneq ($(filter 12 13,$(AVR_LIBSTDCPP_GCC)),)
CXXFLAGS += --param=min-pagesize=0
endif
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections
LDLIBS := -lm

vpath %.cpp $(EXAMPLES)/common $(EXAMPLES)/../runtime
vpath %.cc $(AVR_LIBSTDCPP_DIR)/src

.PHONY: all sim program clean
# Skip examples that require a newer C++ standard than the compiler supports
ifneq ($(shell test $(STD:c++%=%) -ge $(MIN_STD:c++%=%) && echo yes),yes)
all sim program:
	@echo "Skipping $(NAME): requires $(MIN_STD), but $(AVR_CXX) only supports $(STD)"
else
all: $(ELF)
	@avr-size $(ELF)

$(BUILD)/%.o: %
	@mkdir -p $(BUILD)
	$(AVR_CXX) $(CXXFLAGS) -c $< -o $@

$(ELF): $(OBJECTS)
	$(AVR_CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

sim: $(ELF)
	simavr -m $(MCU) -f $(F_CPU) $<

program: $(ELF)
	avr-objcopy -O ihex -R .eeprom $< $(BUILD)/$(NAME).hex
	avrdude -p$(MCU) $(AVRDUDE_FLAGS) -Uflash:w:$(BUILD)/$(NAME).hex:i

endif

clean:
	rm -rf build
