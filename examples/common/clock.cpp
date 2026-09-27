/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// libstdc++ declares std::chrono::steady_clock::now(), but only the
// application knows which hardware timer to use. Here Timer0 generates an
// interrupt every millisecond, which is also the resolution of the clock.

#include <avr/interrupt.h>
#include <avr/io.h>
#include <chrono>
#include <util/atomic.h>

static volatile uint32_t clock_ms{0};

ISR(TIMER0_COMPA_vect)
{
	clock_ms = clock_ms + 1;
}

__attribute__((constructor)) static void
clock_init()
{
	static_assert(F_CPU / 64 / 1000 <= 256, "Timer0 cannot generate 1ms at this F_CPU");
	TCCR0A = _BV(WGM01);				// Clear timer on compare match
	OCR0A = F_CPU / 64 / 1000 - 1;		// 1kHz
	TIMSK0 = _BV(OCIE0A);
	TCCR0B = _BV(CS01) | _BV(CS00);		// Prescaler 64
	sei();
}

std::chrono::steady_clock::time_point
std::chrono::steady_clock::now() noexcept
{
	uint32_t now;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { now = clock_ms; }
	return time_point{std::chrono::milliseconds{now}};
}
