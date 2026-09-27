#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 14
#include "../gcc15/include/stdckdint.h"
#elif __GNUC__ >= 13
#error "<stdckdint.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 11
#error "<stdckdint.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 8
#error "<stdckdint.h> is not available with this avr-gcc version"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
