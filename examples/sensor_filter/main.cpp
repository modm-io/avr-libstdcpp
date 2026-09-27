/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Measures a temperature with an NTC thermistor on pin A0.
//
// Wiring: 5V -- 10k resistor -- A0 -- 10k NTC (B=3950) -- GND
//
// Each measurement takes the median of five ADC samples to remove spikes,
// smooths the result with a moving average and converts it to degrees
// Celsius with the logarithm from <cmath>.

#include <algorithm>
#include <array>
#include <avr/io.h>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <numeric>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

template<typename T, size_t N>
class MovingAverage
{
public:
	T
	update(T value)
	{
		samples[index] = value;
		index = (index + 1) % N;
		count = std::min(count + 1, N);
		return std::accumulate(samples.begin(), samples.begin() + count, uint32_t{0}) / count;
	}

private:
	std::array<T, N> samples{};
	size_t index{0};
	size_t count{0};
};

template<size_t N>
static uint16_t
median(std::array<uint16_t, N> window)
{
	std::nth_element(window.begin(), window.begin() + N / 2, window.end());
	return window[N / 2];
}

static uint16_t
read_adc()
{
	ADCSRA |= _BV(ADSC);
	loop_until_bit_is_clear(ADCSRA, ADSC);
	return ADC;
}

// Beta parameter equation: 1/T = 1/T0 + ln(R/R0)/B
static float
temperature(uint16_t adc)
{
	constexpr float B{3950}, T0{298.15f}, R0{10e3f};
	adc = std::clamp<uint16_t>(adc, 1, 1022);	// avoid division by zero
	const float resistance = R0 * adc / (1023 - adc);
	return 1 / (1 / T0 + std::log(resistance / R0) / B) - 273.15f;
}

int main()
{
	ADMUX = _BV(REFS0);							// AVcc reference, channel 0
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	// 125kHz

	MovingAverage<uint16_t, 8> average;
	auto next = Clock::now();
	for (;;)
	{
		if (Clock::now() < next) continue;
		next += 250ms;

		std::array<uint16_t, 5> window;
		std::generate(window.begin(), window.end(), read_adc);
		const uint16_t adc = average.update(median(window));

		// avr-libc printf() does not support floats by default
		const int16_t decicelsius = std::lround(temperature(adc) * 10);
		printf("adc=%4u  %d.%u C\n", adc, decicelsius / 10, unsigned(std::abs(decicelsius % 10)));
	}
}
