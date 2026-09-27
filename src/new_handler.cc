#if __GNUC__ >= 14
#include "../gcc15/src/new_handler.cc"
#elif __GNUC__ >= 13
#include "../gcc13/src/new_handler.cc"
#elif __GNUC__ >= 11
#include "../gcc12/src/new_handler.cc"
#elif __GNUC__ >= 8
#include "../gcc10/src/new_handler.cc"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
