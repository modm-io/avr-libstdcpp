/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Blinks the LED of an Arduino Uno and prints the uptime every second.
//
// All time calculations use std::chrono, so units are checked by the compiler
// and conversions between them are exact. The steady_clock is implemented in
// examples/common/clock.cpp using Timer0.

#include <avr/io.h>
#include <chrono>
#include <cstdio>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

int main()
{
	DDRB |= _BV(PB5);

	auto next_toggle = Clock::now();
	auto next_report = Clock::now();
	for (;;)
	{
		const auto now = Clock::now();
		if (now >= next_toggle)
		{
			PINB = _BV(PB5);	// writing to PINx toggles the output
			next_toggle += 500ms;
		}
		if (now >= next_report)
		{
			next_report += 1s;
			const auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
#if __cpp_lib_chrono >= 201907L
			// C++20 splits a duration into hours, minutes and seconds
			const std::chrono::hh_mm_ss time{uptime};
			printf("uptime %02u:%02u:%02u\n", unsigned(time.hours().count()),
				   unsigned(time.minutes().count()), unsigned(time.seconds().count()));
#else
			printf("uptime %lus\n", static_cast<unsigned long>(uptime.count()));
#endif
		}
	}
}
