#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 16
#include "../../gcc16/include/bits/simd_mask.h"
#elif __GNUC__ >= 14
#error "<bits/simd_mask.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 13
#error "<bits/simd_mask.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 11
#error "<bits/simd_mask.h> is not available with this avr-gcc version"
#elif __GNUC__ >= 8
#error "<bits/simd_mask.h> is not available with this avr-gcc version"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
