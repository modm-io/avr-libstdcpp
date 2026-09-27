/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// A pedestrian crossing: the traffic light stays green until the button on
// pin D2 is pressed, then it switches to red for the pedestrians.
//
// Wiring: LEDs on D8 (red), D9 (yellow), D10 (green), button from D2 to GND.
//
// Each state is its own type holding only the data it needs, and the current
// state is a std::variant of all of them. std::visit dispatches to the
// transition function of the current state, and the compiler checks that
// every state is handled.

#include <avr/io.h>
#include <chrono>
#include <cstdio>
#include <optional>
#include <variant>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

struct Green { bool requested{false}; };
struct Yellow { Clock::time_point until; };
struct Red { Clock::time_point until; };
using State = std::variant<Green, Yellow, Red>;

// Combines several lambdas into one overloaded function object
template<typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static bool
button_pressed()
{
	return bit_is_clear(PIND, PD2);
}

// Returns the next state or nothing if the state does not change
static std::optional<State>
transition(const State& state, Clock::time_point now)
{
	return std::visit(overloaded
	{
		[&](const Green& green) -> std::optional<State>
		{
			if (green.requested or button_pressed()) return Yellow{now + 2s};
			return std::nullopt;
		},
		[&](const Yellow& yellow) -> std::optional<State>
		{
			if (now >= yellow.until) return Red{now + 5s};
			return std::nullopt;
		},
		[&](const Red& red) -> std::optional<State>
		{
			if (now >= red.until) return Green{};
			return std::nullopt;
		},
	}, state);
}

static void
show(const State& state)
{
	static constexpr const char* names[]{"green", "yellow", "red"};
	static constexpr uint8_t leds[]{_BV(PB2), _BV(PB1), _BV(PB0)};
	PORTB = (PORTB & ~(_BV(PB0) | _BV(PB1) | _BV(PB2))) | leds[state.index()];
	printf("%s\n", names[state.index()]);
}

int main()
{
	DDRB |= _BV(PB0) | _BV(PB1) | _BV(PB2);
	PORTD |= _BV(PD2);

	State state{Green{}};
	show(state);
	for (;;)
	{
		if (auto next = transition(state, Clock::now()))
		{
			state = *next;
			show(state);
		}
	}
}
