#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 14
#include "../../gcc15/include/bits/unicode.h"
#elif __GNUC__ >= 13
#error "<bits/unicode.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 11
#error "<bits/unicode.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 8
#error "<bits/unicode.h> is not available with this avr-gcc version"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
