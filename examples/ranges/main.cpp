/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// A data logger that keeps the last 16 readings and prints statistics about
// the valid ones every second. Requires C++20.
//
// The readings are simulated with <random> and stored in a ring buffer. The
// statistics are computed lazily with range adaptors, which do not allocate
// or copy, and std::span passes the buffer without its size in the type.

#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <cstdio>
#include <random>
#include <ranges>
#include <span>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

struct Reading
{
	int16_t millivolt;
	bool valid;
};

// Accepts any range of integers, for example a view
template<std::ranges::input_range R>
	requires std::integral<std::ranges::range_value_t<R>>
static void
print_all(const char* name, R&& values)
{
	printf("%s:", name);
	for (const auto value : values) printf(" %d", value);
	putchar('\n');
}

static void
report(std::span<const Reading> readings)
{
	auto valid = readings
			| std::views::filter([](const Reading& r) { return r.valid; })
			| std::views::transform([](const Reading& r) { return r.millivolt; });
	if (std::ranges::empty(valid)) return;

	const auto [min, max] = std::ranges::minmax(valid);
	const auto count = std::ranges::distance(valid);
	printf("%d valid, min=%d mV, max=%d mV\n", int(count), min, max);
	print_all("newest", valid | std::views::reverse | std::views::take(4));
}

int main()
{
	std::array<Reading, 16> buffer{};
	size_t index{0};

	std::minstd_rand generator{42};
	std::uniform_int_distribution<int16_t> voltage{0, 5000};
	std::bernoulli_distribution valid{0.8};

	auto next_report = Clock::now() + 1s;
	auto next_sample = Clock::now();
	for (;;)
	{
		const auto now = Clock::now();
		if (now >= next_sample)
		{
			next_sample += 100ms;
			buffer[index] = {voltage(generator), valid(generator)};
			index = (index + 1) % buffer.size();
		}
		if (now >= next_report)
		{
			next_report += 1s;
			// order the ring buffer from oldest to newest
			std::array<Reading, buffer.size()> ordered;
			std::ranges::rotate_copy(buffer, buffer.begin() + index, ordered.begin());
			report(ordered);
		}
	}
}
