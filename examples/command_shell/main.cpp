/*
 * Copyright (c) 2026, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// A command line interface on the serial port (9600 baud).
//
//   > led on
//   > sum 1 2 -3
//   3
//   > uptime
//   12s
//
// The line is split into std::string_view tokens without copying, numbers
// are parsed with std::from_chars and the commands are looked up in a
// std::map of std::function handlers.

#include <algorithm>
#include <avr/io.h>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <string_view>
#include <vector>

using Arguments = std::vector<std::string_view>;

static Arguments
split(std::string_view line)
{
	Arguments tokens;
	while (not line.empty())
	{
		const auto start = line.find_first_not_of(' ');
		if (start == line.npos) break;
		line.remove_prefix(start);
		const auto end = std::min(line.find(' '), line.size());
		tokens.push_back(line.substr(0, end));
		line.remove_prefix(end);
	}
	return tokens;
}

static std::optional<int32_t>
parse(std::string_view token)
{
	int32_t value;
	const auto [end, error] = std::from_chars(token.data(), token.data() + token.size(), value);
	if (error != std::errc{} or end != token.data() + token.size()) return std::nullopt;
	return value;
}

static const std::map<std::string_view, std::function<void(const Arguments&)>> commands
{
	{"led", [](const Arguments& args)
	{
		if (args.size() == 1 and args[0] == "on") PORTB |= _BV(PB5);
		else if (args.size() == 1 and args[0] == "off") PORTB &= ~_BV(PB5);
		else puts("usage: led on|off");
	}},
	{"sum", [](const Arguments& args)
	{
		std::vector<int32_t> values;
		for (const auto arg : args)
		{
			const auto value = parse(arg);
			if (not value) { printf("not a number: %.*s\n", int(arg.size()), arg.data()); return; }
			values.push_back(*value);
		}
		printf("%ld\n", std::accumulate(values.begin(), values.end(), int32_t{0}));
	}},
	{"uptime", [](const Arguments&)
	{
		const auto uptime = std::chrono::steady_clock::now().time_since_epoch();
		printf("%lus\n", static_cast<unsigned long>(
				std::chrono::duration_cast<std::chrono::seconds>(uptime).count()));
	}},
};

int main()
{
	DDRB |= _BV(PB5);
	puts("Commands: led, sum, uptime");

	char buffer[64];
	for (;;)
	{
		fputs("> ", stdout);
		size_t length{0};
		for (int c; (c = getchar()) != '\r' and c != '\n';)
		{
			if (length < sizeof(buffer)) buffer[length++] = c;
			putchar(c);
		}
		putchar('\n');

		const auto tokens = split({buffer, length});
		if (tokens.empty()) continue;
		if (const auto command = commands.find(tokens[0]); command != commands.end())
			command->second({tokens.begin() + 1, tokens.end()});
		else
			printf("unknown command: %.*s\n", int(tokens[0].size()), tokens[0].data());
	}
}
