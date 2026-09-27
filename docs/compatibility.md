# Compatibility

## Why four header sets?

The libstdc++ headers are written for the compiler of the same GCC release.
Newer headers use compiler builtins and language features that older compilers
do not understand, and some compilers cannot parse older headers either.

We generated headers from GCC 9 to 15 and parsed all of them with every
avr-gcc version, which resulted in this matrix (✓ = all headers compile with the newest supported standard):

| avr-gcc | GCC 9 | GCC 10 | GCC 11 | GCC 12 | GCC 13 | GCC 14 | GCC 15 |
|---------|-------|--------|--------|--------|--------|--------|--------|
| 8       | ✓     | C++17  | ✗      | ✗      | ✗      | ✗      | ✗      |
| 9       | ✓     | C++17  | ✗      | ✗      | ✗      | ✗      | ✗      |
| 10      | ✓     | ✓      | ✗      | ✗      | ✗      | ✗      | ✗      |
| 11      | ✓     | ✓      | ✓      | ✓      | ✗      | ✗      | ✗      |
| 12      | ✓     | ✓      | ✓      | ✓      | ✗      | ✗      | ✗      |
| 13      | ✗     | ✗      | ✗      | ✗      | ✓      | ✗      | ✗      |
| 14      | ✓     | ✓      | ✓      | ✓      | ✓      | ✓      | ✓      |
| 15      | ✓     | ✓      | ✓      | ✓      | ✓      | ✓      | ✓      |

Some notable failures:

- avr-gcc ≤ 12 cannot parse the GCC 13 headers, which use new type trait
  builtins such as `__is_convertible`.
- avr-gcc 13 cannot parse older headers, which use `__remove_cv` as an
  identifier, nor newer headers ("explicit template argument list not allowed").
- avr-gcc 8 and 9 do not implement concepts, which the GCC 10 headers require
  in C++20 mode.

To provide the newest library features for every compiler, avr-libstdcpp uses
the newest headers each compiler can parse:

| Header set | Generated from | Supports avr-gcc | C++ standards        |
|------------|----------------|------------------|----------------------|
| `gcc10`    | GCC 10.5       | 8, 9, 10         | C++17 (8, 9), C++20  |
| `gcc12`    | GCC 12.5       | 11, 12           | C++17 to C++23       |
| `gcc13`    | GCC 13.4       | 13               | C++17 to C++23       |
| `gcc15`    | GCC 15.2       | 14, 15           | C++17 to C++26       |

Each set is generated from the GCC release of the newest avr-gcc it supports,
see [generating the header sets](generating.md). The `include` and `src`
directories redirect every file to the set of your avr-gcc version, so you only
need this one include path for all compilers.

avr-gcc 7 and older are not supported. There are no Homebrew builds to test
them, and the previous version of this library required large backports of
`<type_traits>` for avr-gcc 7.


## Tested configurations

The [integration tests](testing.md) are compiled with every supported avr-gcc
version with every C++ standard from C++17 to the newest supported one, and then run in simavr.
We use the avr-gcc builds from [Homebrew](https://github.com/osx-cross/homebrew-avr),
which ship with these avr-libc versions:

| avr-gcc | avr-libc |
|---------|----------|
| 8.5     | 2.1.0    |
| 9.4     | 2.1.0    |
| 10.5    | 2.1.0    |
| 11.5    | 2.0.0    |
| 12.5    | 2.2.1    |
| 13.4    | 2.2.1    |
| 14.3    | 2.2.1    |
| 15.2    | 2.2.1    |


## Notes for specific versions

### avr-gcc 8 and 9

Only C++17 is supported. Their experimental C++2a mode does not implement
concepts. The `[[likely]]` and `[[unlikely]]` attributes are removed from the
headers, since avr-gcc 8 cannot parse them after `if` statements.

`long double` is only 32-bit wide, and avr-libc < 2.2 does not implement
`floorl()` and `ceill()`, which the unordered containers need, so
`src/math.cc` provides them.

### avr-gcc 10

`<coroutine>` requires the `-fcoroutines` flag. avr-gcc 10.1 crashes with an
internal compiler error in `<ranges>` with C++20, use a newer avr-gcc 10 or
C++17.

### avr-gcc 11

avr-gcc 11 cannot parse the C++23 decay-copy `auto(x)`, which GCC 12.5
backported into `std::string::resize_and_overwrite()`. The `gcc12` header set
uses `std::decay_t<decltype(x)>(x)` instead.

### avr-gcc 12 and 13

avr-gcc 12 and 13 warn about every access to a register with `-Warray-bounds`,
see [GCC bug 105523](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523).
Compile with `--param=min-pagesize=0` to avoid this.

### avr-gcc 13

avr-gcc 13 and 14.1 crash with an internal compiler error when resolving
overloads with `std::float32_t` in C++23, since `float` and `double` are both
32-bit on AVR. The `gcc13` and `gcc15` header sets therefore undefine
`__STDCPP_FLOAT32_T__` for these compilers, so `std::float32_t` is not
available.

### Calendar and time zones

The calendar types of `<chrono>` (`year_month_day`, `weekday`, `hh_mm_ss`, …)
are available since the `gcc12` header set, however, `__cpp_lib_chrono` only
announces their complete implementation since GCC 15. Time zones require the
time zone database and are not supported.

### avr-libc

- avr-libc < 2.1 defines the float math functions (`sinf()`, …) as macros for
  the double functions. `src/math.cc` implements them as real functions and
  the headers undefine the macros and declare the functions instead.
- avr-libc < 2.2 does not declare some long double functions, for example
  `truncl()`. The headers declare them, but only `floorl()` and `ceill()` are
  implemented by `src/math.cc`.
- avr-libc 2.1 only implements the float math functions and declares the
  double functions as assembler aliases, which the compiler builtins used by
  the headers ignore. `src/math.cc` implements `ceil()`, `floor()` and `fabs()`
  for them.
- avr-libc does not implement all C99 math functions, for example `acosh()` or
  `tgamma()`. You can call them, but linking fails.
