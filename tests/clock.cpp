/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Deterministic clocks for testing: every call advances time by 1ms
#include <chrono>

std::chrono::steady_clock::time_point
std::chrono::steady_clock::now() noexcept
{
	static time_point now{};
	return now += milliseconds{1};
}

std::chrono::system_clock::time_point
std::chrono::system_clock::now() noexcept
{
	static time_point now{};
	return now += milliseconds{1};
}
