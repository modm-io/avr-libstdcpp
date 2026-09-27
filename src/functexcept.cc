#if __GNUC__ >= 14
#include "../gcc15/src/functexcept.cc"
#elif __GNUC__ >= 13
#include "../gcc13/src/functexcept.cc"
#elif __GNUC__ >= 11
#include "../gcc12/src/functexcept.cc"
#elif __GNUC__ >= 8
#include "../gcc10/src/functexcept.cc"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
