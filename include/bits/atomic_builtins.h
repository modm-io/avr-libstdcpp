/*
 * Copyright (c) 2024, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#pragma once
#include <avr/interrupt.h>

/* We are implementing the libary interface described here:
 * See https://gcc.gnu.org/wiki/Atomic/GCCMM/LIbrary
 *
 * This file is an adaptation of this template in modm:
 * https://github.com/modm-io/modm/blob/develop/ext/gcc/modm_atomic.hpp.in
 *
 * AVRs have no memory barrier instruction, but we still need a compiler fence!
 */

class __modm_atomic_lock
{
public:
	[[gnu::always_inline]] inline  __modm_atomic_lock() : sreg(SREG)
	{
		__atomic_signal_fence(__ATOMIC_SEQ_CST);
		cli();
	}

	[[gnu::always_inline]] inline  ~__modm_atomic_lock()
	{
		SREG = sreg;
		__atomic_signal_fence(__ATOMIC_SEQ_CST);
	}

private:
	uint8_t sreg;
};

// ============================= generic integers =============================
template<typename T>
[[gnu::always_inline]] inline T
__modm_atomic_load_t(const volatile void *ptr)
{
	__modm_atomic_lock l;
	return *reinterpret_cast<const volatile T*>(ptr);
}

template<typename T>
[[gnu::always_inline]] inline void
__modm_atomic_store_t(volatile void *ptr, T value)
{
	__modm_atomic_lock l;
	*reinterpret_cast<volatile T*>(ptr) = value;
}

template<typename T>
[[gnu::always_inline]] inline T
__modm_atomic_exchange_t(volatile void *ptr, T desired)
{
	__modm_atomic_lock l;
	T previous = *reinterpret_cast<volatile T*>(ptr);
	*reinterpret_cast<volatile T*>(ptr) = desired;
	return previous;
}

template<typename T>
[[gnu::always_inline]] inline bool
__modm_atomic_compare_exchange_t(volatile void *ptr, void *expected, T desired)
{
	__modm_atomic_lock l;
	const T current = *reinterpret_cast<volatile T*>(ptr);
	if (current == *reinterpret_cast<T*>(expected)) [[likely]]
	{
		*reinterpret_cast<volatile T*>(ptr) = desired;
		return true;
	}
	else *reinterpret_cast<T*>(expected) = current;
	return false;
}

extern "C"
{

// ================================ lock free =================================
[[gnu::always_inline]] inline bool
__atomic_is_lock_free (unsigned int object_size, const volatile void */*ptr*/)
{
	// only lock free if size ≤ bus width and then also properly aligned
	return (object_size <= 1);
}


// ========================= atomics for 8 bit integers =========================
[[gnu::always_inline]] inline uint8_t
__atomic_exchange_1(volatile void *ptr, uint8_t desired, int /*memorder*/)
{
	return __modm_atomic_exchange_t<uint8_t>(ptr, desired);
}

[[gnu::always_inline]] inline bool
__atomic_compare_exchange_1(volatile void *ptr, void *expected, uint8_t desired,
			bool /*weak*/, int /*success_memorder*/, int /*failure_memorder*/)
{
	return __modm_atomic_compare_exchange_t<uint8_t>(ptr, expected, desired);
}

[[gnu::always_inline]] inline uint8_t
__atomic_fetch_and_1(volatile void *ptr, uint8_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint8_t previous = *reinterpret_cast<volatile uint8_t*>(ptr);
	*reinterpret_cast<volatile uint8_t*>(ptr) = (previous & value);
	return previous;
}

[[gnu::always_inline]] inline uint8_t
__atomic_fetch_or_1(volatile void *ptr, uint8_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint8_t previous = *reinterpret_cast<volatile uint8_t*>(ptr);
	*reinterpret_cast<volatile uint8_t*>(ptr) = (previous | value);
	return previous;
}

[[gnu::always_inline]] inline uint8_t
__atomic_fetch_xor_1(volatile void *ptr, uint8_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint8_t previous = *reinterpret_cast<volatile uint8_t*>(ptr);
	*reinterpret_cast<volatile uint8_t*>(ptr) = (previous ^ value);
	return previous;
}

[[gnu::always_inline]] inline uint8_t
__atomic_fetch_nand_1(volatile void *ptr, uint8_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint8_t previous = *reinterpret_cast<volatile uint8_t*>(ptr);
	*reinterpret_cast<volatile uint8_t*>(ptr) = ~(previous & value);
	return previous;
}

// ========================= atomics for 16 bit integers =========================
[[gnu::always_inline]] inline uint16_t
__atomic_load_2(const volatile void *ptr, int /*memorder*/)
{
	return __modm_atomic_load_t<uint16_t>(ptr);
}

[[gnu::always_inline]] inline void
__atomic_store_2(volatile void *ptr, uint16_t value, int /*memorder*/)
{
	__modm_atomic_store_t<uint16_t>(ptr, value);
}

[[gnu::always_inline]] inline uint16_t
__atomic_exchange_2(volatile void *ptr, uint16_t desired, int /*memorder*/)
{
	return __modm_atomic_exchange_t<uint16_t>(ptr, desired);
}

[[gnu::always_inline]] inline bool
__atomic_compare_exchange_2(volatile void *ptr, void *expected, uint16_t desired,
			bool /*weak*/, int /*success_memorder*/, int /*failure_memorder*/)
{
	return __modm_atomic_compare_exchange_t<uint16_t>(ptr, expected, desired);
}

[[gnu::always_inline]] inline uint16_t
__atomic_fetch_and_2(volatile void *ptr, uint16_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint16_t previous = *reinterpret_cast<volatile uint16_t*>(ptr);
	*reinterpret_cast<volatile uint16_t*>(ptr) = (previous & value);
	return previous;
}

[[gnu::always_inline]] inline uint16_t
__atomic_fetch_or_2(volatile void *ptr, uint16_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint16_t previous = *reinterpret_cast<volatile uint16_t*>(ptr);
	*reinterpret_cast<volatile uint16_t*>(ptr) = (previous | value);
	return previous;
}

[[gnu::always_inline]] inline uint16_t
__atomic_fetch_xor_2(volatile void *ptr, uint16_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint16_t previous = *reinterpret_cast<volatile uint16_t*>(ptr);
	*reinterpret_cast<volatile uint16_t*>(ptr) = (previous ^ value);
	return previous;
}

[[gnu::always_inline]] inline uint16_t
__atomic_fetch_nand_2(volatile void *ptr, uint16_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint16_t previous = *reinterpret_cast<volatile uint16_t*>(ptr);
	*reinterpret_cast<volatile uint16_t*>(ptr) = ~(previous & value);
	return previous;
}

// ========================= atomics for 32 bit integers =========================
[[gnu::always_inline]] inline uint32_t
__atomic_load_4(const volatile void *ptr, int /*memorder*/)
{
	return __modm_atomic_load_t<uint32_t>(ptr);
}

[[gnu::always_inline]] inline void
__atomic_store_4(volatile void *ptr, uint32_t value, int /*memorder*/)
{
	__modm_atomic_store_t<uint32_t>(ptr, value);
}

[[gnu::always_inline]] inline uint32_t
__atomic_exchange_4(volatile void *ptr, uint32_t desired, int /*memorder*/)
{
	return __modm_atomic_exchange_t<uint32_t>(ptr, desired);
}

[[gnu::always_inline]] inline bool
__atomic_compare_exchange_4(volatile void *ptr, void *expected, uint32_t desired,
			bool /*weak*/, int /*success_memorder*/, int /*failure_memorder*/)
{
	return __modm_atomic_compare_exchange_t<uint32_t>(ptr, expected, desired);
}

[[gnu::always_inline]] inline uint32_t
__atomic_fetch_and_4(volatile void *ptr, uint32_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint32_t previous = *reinterpret_cast<volatile uint32_t*>(ptr);
	*reinterpret_cast<volatile uint32_t*>(ptr) = (previous & value);
	return previous;
}

[[gnu::always_inline]] inline uint32_t
__atomic_fetch_or_4(volatile void *ptr, uint32_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint32_t previous = *reinterpret_cast<volatile uint32_t*>(ptr);
	*reinterpret_cast<volatile uint32_t*>(ptr) = (previous | value);
	return previous;
}

[[gnu::always_inline]] inline uint32_t
__atomic_fetch_xor_4(volatile void *ptr, uint32_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint32_t previous = *reinterpret_cast<volatile uint32_t*>(ptr);
	*reinterpret_cast<volatile uint32_t*>(ptr) = (previous ^ value);
	return previous;
}

[[gnu::always_inline]] inline uint32_t
__atomic_fetch_nand_4(volatile void *ptr, uint32_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint32_t previous = *reinterpret_cast<volatile uint32_t*>(ptr);
	*reinterpret_cast<volatile uint32_t*>(ptr) = ~(previous & value);
	return previous;
}

// ========================= atomics for 64 bit integers =========================
[[gnu::always_inline]] inline uint64_t
__atomic_load_8(const volatile void *ptr, int /*memorder*/)
{
	return __modm_atomic_load_t<uint64_t>(ptr);
}

[[gnu::always_inline]] inline void
__atomic_store_8(volatile void *ptr, uint64_t value, int /*memorder*/)
{
	__modm_atomic_store_t<uint64_t>(ptr, value);
}

[[gnu::always_inline]] inline uint64_t
__atomic_exchange_8(volatile void *ptr, uint64_t desired, int /*memorder*/)
{
	return __modm_atomic_exchange_t<uint64_t>(ptr, desired);
}

[[gnu::always_inline]] inline bool
__atomic_compare_exchange_8(volatile void *ptr, void *expected, uint64_t desired,
			bool /*weak*/, int /*success_memorder*/, int /*failure_memorder*/)
{
	return __modm_atomic_compare_exchange_t<uint64_t>(ptr, expected, desired);
}

[[gnu::always_inline]] inline uint64_t
__atomic_fetch_and_8(volatile void *ptr, uint64_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint64_t previous = *reinterpret_cast<volatile uint64_t*>(ptr);
	*reinterpret_cast<volatile uint64_t*>(ptr) = (previous & value);
	return previous;
}

[[gnu::always_inline]] inline uint64_t
__atomic_fetch_or_8(volatile void *ptr, uint64_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint64_t previous = *reinterpret_cast<volatile uint64_t*>(ptr);
	*reinterpret_cast<volatile uint64_t*>(ptr) = (previous | value);
	return previous;
}

[[gnu::always_inline]] inline uint64_t
__atomic_fetch_xor_8(volatile void *ptr, uint64_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint64_t previous = *reinterpret_cast<volatile uint64_t*>(ptr);
	*reinterpret_cast<volatile uint64_t*>(ptr) = (previous ^ value);
	return previous;
}

[[gnu::always_inline]] inline uint64_t
__atomic_fetch_nand_8(volatile void *ptr, uint64_t value, int /*memorder*/)
{
	__modm_atomic_lock l;
	uint64_t previous = *reinterpret_cast<volatile uint64_t*>(ptr);
	*reinterpret_cast<volatile uint64_t*>(ptr) = ~(previous & value);
	return previous;
}

} // extern "C"
