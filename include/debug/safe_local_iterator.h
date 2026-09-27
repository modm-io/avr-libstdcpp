#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 14
#include "../../gcc15/include/debug/safe_local_iterator.h"
#elif __GNUC__ >= 13
#include "../../gcc13/include/debug/safe_local_iterator.h"
#elif __GNUC__ >= 11
#include "../../gcc12/include/debug/safe_local_iterator.h"
#elif __GNUC__ >= 8
#include "../../gcc10/include/debug/safe_local_iterator.h"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
