NAME=numeric-test
MCU=atmega328p

ifeq ($(STD),)
STD=c++17
endif

BUILD_DIR=./build
LIB_DIR=../..
AVR_CXX ?= avr-g++
include $(LIB_DIR)/avr-libstdcpp.mk

ifeq ($(INC),)
INCLUDES= -I$(AVR_LIBSTDCPP_INCLUDE)
else
INCLUDES= -I$(INC)
endif

SOURCES=$(LIB_DIR)/examples/numeric/numeric.cpp $(AVR_LIBSTDCPP_SOURCES)
VPATH=.:$(LIB_DIR)/examples/numeric:$(AVR_LIBSTDCPP_DIR)/src
OBJECTS=$(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:%=%.o)))

WARNFLAGS=-Wall -Wextra -pedantic

ifeq ($(STD),c++20)
WARNFLAGS+=-Wno-volatile
endif

CXXFLAGS=-std=$(STD) -O2 $(WARNFLAGS) -fno-exceptions -fno-rtti -fno-unwind-tables -fno-threadsafe-statics -Wshadow -Wcast-qual -Wpointer-arith -Wundef
LDFLAGS=

TARGET=$(BUILD_DIR)/$(NAME)

all: hex size

hex: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -j .data -j .text $(TARGET).elf $(TARGET).hex

$(TARGET).elf: $(OBJECTS)
	$(AVR_CXX) $(LDFLAGS) -mmcu=$(MCU) $(OBJECTS) -o $(TARGET).elf

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(AVR_CXX) -c $(CXXFLAGS) -mmcu=$(MCU) $(INCLUDES) $< -o $@

$(BUILD_DIR)/%.cc.o: %.cc
	@mkdir -p $(BUILD_DIR)
	$(AVR_CXX) -c $(CXXFLAGS) -mmcu=$(MCU) $(INCLUDES) $< -o $@

size: $(TARGET).elf
	avr-objdump -Pmem-usage $(TARGET).elf

program: $(TARGET).hex
	avrdude -p$(MCU) $(AVRDUDE_FLAGS) -c$(PROGRAMMER) -Uflash:w:$(TARGET).hex:a

clean:
	rm -rf $(BUILD_DIR)/*.o
	rm -rf $(BUILD_DIR)/*.elf
	rm -rf $(BUILD_DIR)/*.hex
