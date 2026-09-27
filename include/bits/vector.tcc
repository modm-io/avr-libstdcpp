#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 14
#include "../../gcc15/include/bits/vector.tcc"
#elif __GNUC__ >= 13
#include "../../gcc13/include/bits/vector.tcc"
#elif __GNUC__ >= 11
#include "../../gcc12/include/bits/vector.tcc"
#elif __GNUC__ >= 8
#include "../../gcc10/include/bits/vector.tcc"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
