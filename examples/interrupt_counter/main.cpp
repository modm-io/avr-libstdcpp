/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Counts falling edges on pin D2 (INT0) and prints the rate every second.
//
// The counter is shared between the interrupt and the main loop, so it must
// be a std::atomic. AVR does not nest interrupts, so the interrupt can simply
// load and store the counter: nothing can interrupt it in between. Relaxed
// loads and stores of a single byte compile to plain instructions.
//
// The main loop however can be interrupted, so it must read and reset the
// counter in one atomic operation, otherwise an edge could be lost in between.
// AVR has no atomic instructions, so avr-gcc calls __atomic_exchange_1(),
// implemented in runtime/atomic.cpp by briefly disabling interrupts.

#include <atomic>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <chrono>
#include <cstdio>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

// A single byte, so that the interrupt can access it without a function call.
// The count saturates at 255 edges per second.
static std::atomic<uint8_t> edges{0};

ISR(INT0_vect)
{
	// Not a fetch_add(), since the interrupt cannot be interrupted anyway
	const uint8_t count = edges.load(std::memory_order_relaxed);
	if (count < 255) edges.store(count + 1, std::memory_order_relaxed);
}

int main()
{
	PORTD |= _BV(PD2);		// pull-up, so a button can connect D2 to GND
	EICRA = _BV(ISC01);		// interrupt on falling edge
	EIMSK = _BV(INT0);
	sei();

	auto next_report = Clock::now() + 1s;
	for (;;)
	{
		if (Clock::now() >= next_report)
		{
			next_report += 1s;
			printf("%u edges/s\n", edges.exchange(0));
		}
	}
}
