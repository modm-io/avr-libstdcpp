#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 14
#error "<bits/range_cmp.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 13
#error "<bits/range_cmp.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 11
#error "<bits/range_cmp.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 8
#include "../../gcc10/include/bits/range_cmp.h"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
