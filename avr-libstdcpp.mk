# Copyright (c) 2026, Niklas Hauser
#
# This file is part of the modm project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# The headers and sources select the header set for your avr-gcc version
# themselves. This file only provides the paths and the newest supported C++
# standard. Set AVR_CXX to your avr-g++ before including this file, then use:
#
#   AVR_LIBSTDCPP_INCLUDE  Include path, add it with -I
#   AVR_LIBSTDCPP_SOURCES  Library sources, compile and link them
#   AVR_LIBSTDCPP_STD      Newest C++ standard supported by $(AVR_CXX)
#
# See docs/getting-started.md for details.

AVR_CXX ?= avr-g++
AVR_LIBSTDCPP_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
AVR_LIBSTDCPP_INCLUDE := $(AVR_LIBSTDCPP_DIR)/include
AVR_LIBSTDCPP_SOURCES := $(wildcard $(AVR_LIBSTDCPP_DIR)/src/*.cc)
AVR_LIBSTDCPP_GCC := $(shell $(AVR_CXX) -dumpversion | cut -d. -f1)

ifneq ($(filter 8 9,$(AVR_LIBSTDCPP_GCC)),)
AVR_LIBSTDCPP_STD := c++17
else ifeq ($(AVR_LIBSTDCPP_GCC),10)
AVR_LIBSTDCPP_STD := c++20
else ifneq ($(filter 11 12 13,$(AVR_LIBSTDCPP_GCC)),)
AVR_LIBSTDCPP_STD := c++23
else ifneq ($(filter 14 15,$(AVR_LIBSTDCPP_GCC)),)
AVR_LIBSTDCPP_STD := c++26
else
$(error avr-libstdcpp does not support avr-gcc '$(AVR_LIBSTDCPP_GCC)' ($(AVR_CXX)))
endif
