#if __GNUC__ >= 14
#include "../gcc15/src/math.cc"
#elif __GNUC__ >= 13
#include "../gcc13/src/math.cc"
#elif __GNUC__ >= 11
#include "../gcc12/src/math.cc"
#elif __GNUC__ >= 8
#include "../gcc10/src/math.cc"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
