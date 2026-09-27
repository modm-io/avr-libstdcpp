/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// AVR has no atomic instructions, so avr-gcc calls these library functions
// for every std::atomic operation. They are made atomic by disabling
// interrupts, which is sufficient on a single core.

#include <avr/io.h>
#include <stdint.h>

// GCC 12 wrongly warns about accessing SREG via its constant address
#pragma GCC diagnostic ignored "-Warray-bounds"

namespace
{
struct InterruptLock
{
	uint8_t sreg{SREG};
	InterruptLock() { __asm__ volatile ("cli" ::: "memory"); }
	~InterruptLock() { SREG = sreg; __asm__ volatile ("" ::: "memory"); }
};
}

#define AVR_LIBSTDCPP_ATOMIC_OP(N, T, NAME, OP) \
	extern "C" T __atomic_fetch_##NAME##_##N(volatile void* ptr, T value, int) \
	{ InterruptLock lock; auto p = static_cast<volatile T*>(ptr); T old = *p; *p = old OP value; return old; } \
	extern "C" T __atomic_##NAME##_fetch_##N(volatile void* ptr, T value, int) \
	{ InterruptLock lock; auto p = static_cast<volatile T*>(ptr); T result = *p OP value; *p = result; return result; }

#define AVR_LIBSTDCPP_ATOMIC(N, T) \
	extern "C" T __atomic_load_##N(const volatile void* ptr, int) \
	{ InterruptLock lock; return *static_cast<const volatile T*>(ptr); } \
	extern "C" void __atomic_store_##N(volatile void* ptr, T value, int) \
	{ InterruptLock lock; *static_cast<volatile T*>(ptr) = value; } \
	extern "C" T __atomic_exchange_##N(volatile void* ptr, T value, int) \
	{ InterruptLock lock; auto p = static_cast<volatile T*>(ptr); T old = *p; *p = value; return old; } \
	extern "C" bool __atomic_compare_exchange_##N(volatile void* ptr, void* expected, \
												 T desired, bool, int, int) \
	{ \
		InterruptLock lock; \
		auto p = static_cast<volatile T*>(ptr); \
		auto e = static_cast<T*>(expected); \
		if (*p == *e) { *p = desired; return true; } \
		*e = *p; return false; \
	} \
	AVR_LIBSTDCPP_ATOMIC_OP(N, T, add, +) \
	AVR_LIBSTDCPP_ATOMIC_OP(N, T, sub, -) \
	AVR_LIBSTDCPP_ATOMIC_OP(N, T, and, &) \
	AVR_LIBSTDCPP_ATOMIC_OP(N, T, or, |) \
	AVR_LIBSTDCPP_ATOMIC_OP(N, T, xor, ^)

AVR_LIBSTDCPP_ATOMIC(1, uint8_t)
AVR_LIBSTDCPP_ATOMIC(2, uint16_t)
AVR_LIBSTDCPP_ATOMIC(4, uint32_t)
AVR_LIBSTDCPP_ATOMIC(8, uint64_t)
