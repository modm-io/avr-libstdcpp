#if __GNUC__ >= 14
#include "../gcc15/src/hashtable_c++0x.cc"
#elif __GNUC__ >= 13
#include "../gcc13/src/hashtable_c++0x.cc"
#elif __GNUC__ >= 11
#include "../gcc12/src/hashtable_c++0x.cc"
#elif __GNUC__ >= 8
#include "../gcc10/src/hashtable_c++0x.cc"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
