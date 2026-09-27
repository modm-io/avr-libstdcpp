/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Fades the LED on pin D9 in and out along a sine wave.
//
// The sine table is computed by the compiler with a constexpr function that
// returns a std::array, so there is no floating-point math at runtime. The
// table is stored in flash with PROGMEM, since the ATmega328P only has 2kB RAM.

#include <array>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <chrono>
#include <cstdint>
#include <cstdio>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

constexpr double pi = 3.14159265358979323846;

// std::sin is only constexpr since C++26, so use a Taylor series instead
constexpr double
sine(double x)
{
	double term = x, sum = x;
	for (int n = 1; n < 10; n++)
	{
		term *= -x * x / ((2 * n) * (2 * n + 1));
		sum += term;
	}
	return sum;
}

template<size_t N>
constexpr std::array<uint8_t, N>
make_sine_table()
{
	static_assert((N & (N - 1)) == 0, "N must be a power of two for fast wrapping");
	std::array<uint8_t, N> table{};
	for (size_t i = 0; i < N; i++)
		table[i] = static_cast<uint8_t>(127.5 + 127.5 * sine(2 * pi * i / N));
	return table;
}

constexpr auto sine_table = make_sine_table<64>();
static_assert(sine_table[0] == 127 and sine_table[16] == 255 and sine_table[48] == 0);

// A copy of the table in flash, read with pgm_read_byte()
static const std::array<uint8_t, sine_table.size()> sine_flash PROGMEM = sine_table;

int main()
{
	DDRB |= _BV(PB1);
	TCCR1A = _BV(COM1A1) | _BV(WGM10);		// 8-bit fast PWM on OC1A
	TCCR1B = _BV(WGM12) | _BV(CS10);

	printf("sine table in flash: %u bytes\n", unsigned(sizeof(sine_flash)));

	uint8_t phase{0};
	auto next = Clock::now();
	for (;;)
	{
		if (Clock::now() < next) continue;
		next += 30ms;
		OCR1A = pgm_read_byte(&sine_flash[phase]);
		phase = (phase + 1) % sine_flash.size();
	}
}
