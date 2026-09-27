# Limitations

avr-libstdcpp contains the complete libstdc++ headers, but only a few sources
of the compiled library. Everything that is implemented in the headers works,
everything that requires the compiled library fails to link.

## What works

Everything that is implemented in templates or inline functions, for example:

- Containers: `array`, `vector`, `deque`, `list`, `forward_list`, `map`,
  `set`, `unordered_map`, `unordered_set`, `queue`, `stack`, `bitset`, `span`
- Strings: `string`, `string_view`, `to_chars`, `from_chars`, `to_string`
- Algorithms and ranges: `<algorithm>`, `<numeric>`, `<ranges>`
- Utilities: `optional`, `variant`, `any`, `expected`, `tuple`, `function`,
  `unique_ptr`, `bit`, `ratio`, `type_traits`, `concepts`, `compare`
- `<chrono>` durations, time points and calendar types
- `<cmath>`, `<complex>`, `<numbers>`, `<random>` (except `random_device`)
- `<atomic>` with [the runtime](getting-started.md#3-provide-the-runtime)
- Coroutines

The [integration tests](../tests) show more examples.

## What does not work

These features require the compiled library, so they fail to link:

- I/O streams (`<iostream>`, `<sstream>`, `<fstream>`)
- Locales (`<locale>`)
- Formatting (`<format>`, `<print>`)
- `std::random_device`
- The time zone database of `<chrono>`
- Threads (`<thread>`, `<mutex>`, `<condition_variable>`): there is no
  operating system
- Filesystem (`<filesystem>`)

These are disabled on purpose:

- Exceptions and RTTI are not supported. Compile with
  `-fno-exceptions -fno-rtti`. Errors that would throw an exception call
  `abort()`, see [getting started](getting-started.md#3-provide-the-runtime).
- The mathematical special functions of C++17 (`std::beta()`,
  `std::legendre()`, …) are disabled, since they are implemented in the TR1
  headers, which are not included.
- Wide characters (`wchar_t`, `<cwchar>`, `<cwctype>`) are not supported by
  avr-libc.
- The `<cfenv>` and `<ctgmath>` headers, since avr-libc has no `<fenv.h>`
  and `<tgmath.h>`.
- Some C99 functions of `<cstdlib>` and `<cstdio>` that avr-libc does not
  implement, such as `std::lldiv()` and `std::vsscanf()`.

## Things to keep in mind

- `int` and `size_t` are only 16-bit wide, which limits the size of containers
  and the range of many algorithms. `-mint8` is not supported.
- `double` is only 32-bit wide by default, the same as `float`. Since avr-gcc 10
  `long double` is 64-bit and is implemented in software by libgcc, which is
  much slower and larger.
- Dynamic memory is very limited: an ATmega328P has only 2kB of RAM. Consider
  containers with a fixed capacity like `std::array`, or custom allocators.
- `avr-libc`'s `printf()` does not support floating-point numbers by default.
- The `__int24` and `__uint24` types of avr-gcc are not recognized by
  `<type_traits>`.
