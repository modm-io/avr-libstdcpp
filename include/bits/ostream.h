#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 14
#include "../../gcc15/include/bits/ostream.h"
#elif __GNUC__ >= 13
#error "<bits/ostream.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 11
#error "<bits/ostream.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 8
#error "<bits/ostream.h> is not available with this avr-gcc version"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
