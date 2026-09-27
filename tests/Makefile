# Copyright (c) 2026, Niklas Hauser
#
# This file is part of the modm project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Integration tests: builds all tests with every C++ standard from C++17 to the
# newest one supported by $(AVR_CXX), then runs them in simavr. See docs/testing.md.
#
#   make                         build and run all tests with avr-g++
#   make AVR_CXX=path/to/avr-g++ use another compiler
#   make build                   only compile and link

AVR_CXX ?= avr-g++
include ../avr-libstdcpp.mk

SIMAVR ?= simavr
TIMEOUT ?= 60
MCU := atmega2560
TESTS := containers utilities modern
STDS := $(shell for s in 17 20 23 26; do [ $$s -le $(AVR_LIBSTDCPP_STD:c++%=%) ] && echo c++$$s; done)

ifndef STD
.PHONY: all build clean
all build:
	@set -e; for std in $(STDS); do $(MAKE) --no-print-directory STD=$$std $@-std; done
clean:
	rm -rf build
else

BUILD := build/gcc$(AVR_LIBSTDCPP_GCC)/$(STD)
CXXFLAGS := -std=$(STD) -Os -mmcu=$(MCU) -I$(AVR_LIBSTDCPP_INCLUDE) -Wall -Wextra -Werror \
            -fno-exceptions -fno-rtti -fno-threadsafe-statics -ffunction-sections -fdata-sections
CXXFLAGS += $(shell $(AVR_CXX) -fcoroutines -E -x c++ /dev/null >/dev/null 2>&1 && echo -fcoroutines)
# Compound assignments to volatile registers are deprecated in C++20 only
CXXFLAGS += -Wno-volatile
# GCC 12 and 13 wrongly warn about every register access (GCC bug 105523)
ifneq ($(filter 12 13,$(AVR_LIBSTDCPP_GCC)),)
CXXFLAGS += --param=min-pagesize=0
endif
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections

vpath %.cc $(AVR_LIBSTDCPP_DIR)/src
vpath %.cpp ../runtime
LIBRARY := $(addprefix $(BUILD)/,$(notdir $(AVR_LIBSTDCPP_SOURCES:=.o)) \
           $(notdir $(addsuffix .o,$(wildcard ../runtime/*.cpp))) clock.cpp.o)

.PHONY: all-std build-std
.SECONDARY:
build-std: $(TESTS:%=$(BUILD)/%.elf)
all-std: $(TESTS:%=$(BUILD)/%.log)

$(BUILD)/%.o: % test.hpp
	@mkdir -p $(BUILD)
	$(AVR_CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.elf: $(BUILD)/%.cpp.o $(LIBRARY)
	$(AVR_CXX) $(LDFLAGS) $^ -o $@ -lm

$(BUILD)/%.log: $(BUILD)/%.elf
	@perl -e 'alarm shift; exec @ARGV' $(TIMEOUT) $(SIMAVR) -m $(MCU) -f 16000000 $< > $@ 2>&1 || true
	@grep -E 'PASSED|FAIL' $@ | sed 's|^|gcc$(AVR_LIBSTDCPP_GCC) $(STD) $*: |'
	@grep -q PASSED $@ || { rm $@; exit 1; }

endif
