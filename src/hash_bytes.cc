#if __GNUC__ >= 14
#include "../gcc15/src/hash_bytes.cc"
#elif __GNUC__ >= 13
#include "../gcc13/src/hash_bytes.cc"
#elif __GNUC__ >= 11
#include "../gcc12/src/hash_bytes.cc"
#elif __GNUC__ >= 8
#include "../gcc10/src/hash_bytes.cc"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
