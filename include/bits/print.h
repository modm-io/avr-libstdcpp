#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 16
#include "../../gcc16/include/bits/print.h"
#elif __GNUC__ >= 14
#error "<bits/print.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 13
#error "<bits/print.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 11
#error "<bits/print.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 8
#error "<bits/print.h> is not available with this avr-gcc version"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
