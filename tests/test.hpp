/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Minimal test harness for simavr: results are printed via USART0 and the
// simulation ends by sleeping with interrupts disabled.
#pragma once

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>

namespace test
{

inline uint16_t checks{0};
inline uint16_t failures{0};

inline void
put(char c)
{
	UCSR0B = _BV(TXEN0);
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

inline void
print(const char* str)
{
	while (*str) put(*str++);
}

inline void
print(uint16_t value)
{
	char buffer[6];
	uint8_t length{0};
	do { buffer[length++] = '0' + value % 10; value /= 10; } while (value);
	while (length) put(buffer[--length]);
}

inline void
check(bool condition, uint16_t line)
{
	checks++;
#ifdef TEST_VERBOSE
	print("line "); print(line); put('\n');
#endif
	if (condition) return;
	failures++;
	print("FAIL line ");
	print(line);
	put('\n');
}

[[noreturn]] inline void
finish()
{
	print(failures ? "FAILED " : "PASSED ");
	print(checks - failures);
	put('/');
	print(checks);
	put('\n');
	cli();
	for (;;) sleep_mode();
}

}	// namespace test

#define TEST_ASSERT(...) test::check((__VA_ARGS__), __LINE__)
