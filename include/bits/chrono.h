#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 14
#include "../../gcc15/include/bits/chrono.h"
#elif __GNUC__ >= 13
#include "../../gcc13/include/bits/chrono.h"
#elif __GNUC__ >= 11
#include "../../gcc12/include/bits/chrono.h"
#elif __GNUC__ >= 8
#error "<bits/chrono.h> is not available with this avr-gcc version"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
