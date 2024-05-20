/*
 * Copyright (c) 2020, 2024, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <bits/atomic_builtins.h>

/* We are implementing the libary interface described here:
 * See https://gcc.gnu.org/wiki/Atomic/GCCMM/LIbrary
 *
 * This file is an adaptation of this template in modm:
 * https://github.com/modm-io/modm/blob/develop/ext/gcc/atomic.cpp.in
 *
 * No need for compiler fences, since the function call already is one.
 */

extern "C"
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

// ============================ atomics for arrays ============================
// These functions cannot be inlined, since the compiler builtins are named the
// same. Terrible design really.
void
__atomic_load(unsigned int size, const volatile void *src, void *dest, int /*memorder*/)
{
	__modm_atomic_lock l;
	__builtin_memcpy(dest, (const void*)src, size);
}

void
__atomic_store(unsigned int size, volatile void *dest, void *src, int /*memorder*/)
{
	__modm_atomic_lock l;
	__builtin_memcpy((void*)dest, src, size);
}

void
__atomic_exchange(unsigned int size, volatile void *ptr, void *val, void *ret, int /*memorder*/)
{
	__modm_atomic_lock l;
	__builtin_memcpy(ret, (void*)ptr, size);
	__builtin_memcpy((void*)ptr, val, size);
}

bool
__atomic_compare_exchange(unsigned int len, volatile void *ptr, void *expected, void *desired,
						  int /*success_memorder*/, int /*failure_memorder*/)
{
	__modm_atomic_lock l;
	if (__builtin_memcmp((void*)ptr, expected, len) == 0) [[likely]]
	{
		__builtin_memcpy((void*)ptr, desired, len);
		return true;
	}
	else __builtin_memcpy(expected, (void*)ptr, len);
	return false;
}

#pragma GCC diagnostic pop

// These functions cannot be inlined since the compiler refuses to find these
// functions even if they are declared right at the call site. Unclear why.

// ========================= atomics for 8 bit integers =========================
uint8_t
__atomic_fetch_add_1(volatile void *ptr, uint8_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint8_t previous = *reinterpret_cast<volatile uint8_t*>(ptr);
	*reinterpret_cast<volatile uint8_t*>(ptr) = (previous + value);
	return previous;
}

uint8_t
__atomic_fetch_sub_1(volatile void *ptr, uint8_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint8_t previous = *reinterpret_cast<volatile uint8_t*>(ptr);
	*reinterpret_cast<volatile uint8_t*>(ptr) = (previous - value);
	return previous;
}

// ========================= atomics for 16 bit integers =========================
uint16_t
__atomic_fetch_add_2(volatile void *ptr, uint16_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint16_t previous = *reinterpret_cast<volatile uint16_t*>(ptr);
	*reinterpret_cast<volatile uint16_t*>(ptr) = (previous + value);
	return previous;
}

uint16_t
__atomic_fetch_sub_2(volatile void *ptr, uint16_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint16_t previous = *reinterpret_cast<volatile uint16_t*>(ptr);
	*reinterpret_cast<volatile uint16_t*>(ptr) = (previous - value);
	return previous;
}

// ========================= atomics for 32 bit integers =========================
uint32_t
__atomic_fetch_add_4(volatile void *ptr, uint32_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint32_t previous = *reinterpret_cast<volatile uint32_t*>(ptr);
	*reinterpret_cast<volatile uint32_t*>(ptr) = (previous + value);
	return previous;
}

uint32_t
__atomic_fetch_sub_4(volatile void *ptr, uint32_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint32_t previous = *reinterpret_cast<volatile uint32_t*>(ptr);
	*reinterpret_cast<volatile uint32_t*>(ptr) = (previous - value);
	return previous;
}

// ========================= atomics for 64 bit integers =========================
uint64_t
__atomic_fetch_add_8(volatile void *ptr, uint64_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint64_t previous = *reinterpret_cast<volatile uint64_t*>(ptr);
	*reinterpret_cast<volatile uint64_t*>(ptr) = (previous + value);
	return previous;
}

uint64_t
__atomic_fetch_sub_8(volatile void *ptr, uint64_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint64_t previous = *reinterpret_cast<volatile uint64_t*>(ptr);
	*reinterpret_cast<volatile uint64_t*>(ptr) = (previous - value);
	return previous;
}

} // extern "C"
