/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// C++20/23/26 library features, each guarded by its feature-test macro
#include "test.hpp"

#include <version>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <vector>
#if __has_include(<bit>)
#include <bit>
#endif
#if __has_include(<span>)
#include <span>
#endif
#if __has_include(<ranges>)
#include <ranges>
#endif
#if __has_include(<numbers>)
#include <numbers>
#endif
#if __has_include(<compare>)
#include <compare>
#endif
#if __has_include(<concepts>)
#include <concepts>
#endif
#ifdef __cpp_lib_source_location
#include <source_location>
#endif
#ifdef __cpp_lib_expected
#include <expected>
#endif
#if __has_include(<coroutine>) && defined(__cpp_impl_coroutine)
#include <coroutine>
#endif

#ifdef __cpp_lib_three_way_comparison
struct Point
{
	int x, y;
	auto operator<=>(const Point&) const = default;
};
#endif

#ifdef __cpp_lib_concepts
template<std::integral T>
constexpr T twice(T v) { return v * 2; }
static_assert(twice(21) == 42);
#endif

#ifdef __cpp_lib_coroutine
struct Generator
{
	struct promise_type
	{
		int value{};
		Generator get_return_object() { return Generator{std::coroutine_handle<promise_type>::from_promise(*this)}; }
		std::suspend_always initial_suspend() noexcept { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		std::suspend_always yield_value(int v) { value = v; return {}; }
		void return_void() {}
		void unhandled_exception() {}
	};
	std::coroutine_handle<promise_type> handle;
	~Generator() { handle.destroy(); }
	int next() { handle.resume(); return handle.promise().value; }
};

static Generator
counter()
{
	for (int i = 0;; ++i) co_yield i;
}
#endif

volatile uint32_t one = 1;

int main()
{
	TEST_ASSERT(__cplusplus >= 201703L);
	using namespace std::chrono;
#if __cpp_lib_chrono >= 201907L
	constexpr year_month_day ymd{2026y, September, 26d};
	static_assert(weekday{sys_days{ymd}} == Saturday);
	constexpr hh_mm_ss<seconds> hms{seconds{3725}};
	static_assert(hms.hours().count() == 1 and hms.minutes().count() == 2);
	const year_month_day later{sys_days{ymd} + days{one * 10}};
	TEST_ASSERT(later.month() == October and later.day() == 6d);
#endif

#ifdef __cpp_lib_bitops
	TEST_ASSERT(std::popcount(uint16_t(0xF0F0 * one)) == 8 and std::countl_zero(uint8_t(one)) == 7);
#endif
#ifdef __cpp_lib_bit_cast
	TEST_ASSERT(std::bit_cast<float>(uint32_t(0x3f800000 * one)) == 1.f);
#endif
#ifdef __cpp_lib_int_pow2
	TEST_ASSERT(std::bit_ceil(17u * one) == 32 and std::bit_width(100u * one) == 7);
#endif

	[[maybe_unused]] std::array<int, 6> arr{5, 2, 8, 1, 9, 3};
#ifdef __cpp_lib_span
	std::span<int> s{arr};
	TEST_ASSERT(s.subspan(1, 3).size() == 3 and s.front() == 5);
#endif
#ifdef __cpp_lib_ranges
	std::ranges::sort(arr);
	TEST_ASSERT((arr == std::array<int, 6>{1, 2, 3, 5, 8, 9}));
	int sum{};
	for (int v : arr | std::views::filter([](int v) { return v % 2 == 0; })
					 | std::views::transform([](int v) { return v * v; }))
		sum += v;
	TEST_ASSERT(sum == 68);
	TEST_ASSERT(std::ranges::distance(std::views::iota(0, 10) | std::views::take(3)) == 3);
#endif
#ifdef __cpp_lib_math_constants
	TEST_ASSERT(std::numbers::pi_v<float> > 3.1415f and std::numbers::pi_v<float> < 3.1416f);
#endif
#ifdef __cpp_lib_three_way_comparison
	TEST_ASSERT((Point{1, 2} < Point{1, 3}) and (Point{2, 0} > Point{1, 9}));
#endif
#ifdef __cpp_lib_constexpr_vector
	constexpr auto total = [] { std::vector<int> v{1, 2, 3}; v.push_back(4); int s = 0; for (int x : v) s += x; return s; }();
	static_assert(total == 10);
#endif
#ifdef __cpp_lib_source_location
	TEST_ASSERT(std::source_location::current().line() == __LINE__);
#endif
#ifdef __cpp_lib_expected
	std::expected<int, uint8_t> e = std::unexpected(uint8_t(3));
	TEST_ASSERT(e.value_or(1) == 1 and e.error() == 3);
#endif
#ifdef __cpp_lib_ranges_zip
	std::array<int, 3> b{1, 2, 3};
	int zipped{};
	for (auto [l, r] : std::views::zip(arr, b)) zipped += l * r;
	TEST_ASSERT(zipped == 1 + 4 + 9);
#endif
#ifdef __cpp_lib_coroutine
	auto gen = counter();
	TEST_ASSERT(gen.next() == 0 and gen.next() == 1);
#endif
	test::finish();
}
