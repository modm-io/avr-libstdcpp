/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Vocabulary types, memory, math, random, atomics and chrono
#include "test.hpp"

#include <any>
#include <array>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <ratio>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

struct Base { virtual ~Base() = default; virtual int get() const = 0; };
struct Derived : Base { int get() const override { return 42; } };

static bool
near(float a, float b)
{
	return std::fabs(a - b) < 1e-4f;
}

template<typename T>
static constexpr bool
classify()
{
	using limits = std::numeric_limits<T>;
	return std::fpclassify(limits::quiet_NaN()) == FP_NAN and
		std::fpclassify(limits::infinity()) == FP_INFINITE and
		std::fpclassify(T(0)) == FP_ZERO and
		std::fpclassify(limits::denorm_min()) == FP_SUBNORMAL and
		std::fpclassify(T(1)) == FP_NORMAL;
}

volatile float half = 0.5f;

int main()
{
	std::function<int(int)> f = [](int x) { return x * 2; };
	auto bound = std::bind(std::plus<int>{}, std::placeholders::_1, 3);
	TEST_ASSERT(f(2) == 4 and bound(4) == 7 and std::invoke(f, 5) == 10);
	std::array<int, 8> big{};
	std::function<int()> heap = [big]() { return int(big.size()); };
	TEST_ASSERT(heap() == 8);

	std::optional<int> o;
	TEST_ASSERT(not o.has_value() and o.value_or(1) == 1);
	o = 5;
	TEST_ASSERT(*o == 5);
	std::variant<int, float, uint8_t> var{2.5f};
	TEST_ASSERT(std::visit([](auto x) { return int(x); }, var) == 2);
	var = uint8_t(3);
	TEST_ASSERT(std::get<uint8_t>(var) == 3 and var.index() == 2 and std::holds_alternative<uint8_t>(var));
	std::any any = 3;
	TEST_ASSERT(std::any_cast<int>(any) == 3 and std::any_cast<float>(&any) == nullptr);
	auto [x, y] = std::make_tuple(1, 2.f);
	TEST_ASSERT(x == 1 and y == 2.f and std::tuple_size_v<std::tuple<int, int, int>> == 3);

	std::unique_ptr<Base> p = std::make_unique<Derived>();
	auto arr = std::make_unique<int[]>(4);
	arr[2] = p->get();
	TEST_ASSERT(arr[2] == 42);

	char buffer[16]{};
	auto result = std::to_chars(buffer, buffer + sizeof(buffer), 12345);
	TEST_ASSERT(result.ptr - buffer == 5 and std::strcmp(buffer, "12345") == 0);
	int parsed{};
	std::from_chars(buffer, result.ptr, parsed);
	TEST_ASSERT(parsed == 12345);
	result = std::to_chars(buffer, buffer + sizeof(buffer), 255, 16);
	TEST_ASSERT(std::string_view(buffer, result.ptr - buffer) == "ff");

	const float angle = half;
	TEST_ASSERT(near(std::sin(angle), 0.479426f) and near(std::sqrt(4 * angle), 1.414214f));
	TEST_ASSERT(near(std::pow(4 * angle, 3.f), 8.f) and std::round(5 * angle) == 3.f);
	TEST_ASSERT(near(std::fmod(11 * angle, 2.f), 1.5f) and near(std::atan2(angle, angle), 0.785398f));
	TEST_ASSERT(std::isnan(std::numeric_limits<float>::quiet_NaN()) and std::signbit(-angle));
	TEST_ASSERT(std::numeric_limits<int16_t>::max() == 32767);
	static_assert(std::is_same_v<std::float_t, float> and std::is_same_v<std::double_t, double>);
	static_assert(classify<float>() and classify<double>() and classify<long double>());
	static_assert(std::fpclassify(0) == FP_ZERO and std::fpclassify(3) == FP_NORMAL);
	TEST_ASSERT(std::fpclassify(angle) == FP_NORMAL and std::isnormal(angle) and std::isfinite(angle));
	std::complex<float> c{1.f, 2.f};
	TEST_ASSERT(near(std::abs(c * c), 5.f));

	std::minstd_rand engine{42};
	TEST_ASSERT(engine() == 2027382);
	std::uniform_int_distribution<int16_t> distribution{0, 100};
	const int16_t random = distribution(engine);
	TEST_ASSERT(random >= 0 and random <= 100);

	std::atomic<uint8_t> a8{0};
	std::atomic<uint16_t> a16{0};
	std::atomic<uint32_t> a32{0};
	a8++;
	TEST_ASSERT(a16.fetch_add(2) == 0 and a16.exchange(5) == 2 and a16.load() == 5);
	uint32_t expected = 0;
	TEST_ASSERT(a32.compare_exchange_strong(expected, 7) and a32 == 7);
	TEST_ASSERT(not a32.compare_exchange_strong(expected, 9) and expected == 7);
	TEST_ASSERT((a8 |= 6) == 7 and (a8 &= 3) == 3);

	using namespace std::chrono;
	const auto start = steady_clock::now();
	TEST_ASSERT(duration_cast<milliseconds>(steady_clock::now() - start).count() == 1);
	TEST_ASSERT(duration_cast<milliseconds>(seconds{2}).count() == 2000);
	TEST_ASSERT((std::ratio_add<std::ratio<1, 2>, std::ratio<1, 3>>::den == 6));
	static_assert(std::is_trivially_copyable_v<std::array<int, 2>>);

	test::finish();
}
