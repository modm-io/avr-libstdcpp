# Generating the header sets

The header sets are not written by hand, but generated from the libstdc++
sources of the GCC releases. This document explains how and why, and how to
add support for a new GCC version.


## How it works

The libstdc++ headers are almost independent of the target. Only
`bits/c++config.h` and a few other files in the target directory describe the
C library and the operating system. They are generated when libstdc++ is
built: its configure script checks which functions the C library provides.
avr-gcc is built without libstdc++, however, the configure script supports
avr-libc, so we run it with avr-gcc and only install the headers.

[`tools/generate.py`](../tools/generate.py) generates the header sets listed
in its `SETS` table, each defined by the newest and the oldest avr-gcc version
that it supports. The newest `avr-g++` determines the GCC release: the headers
of avr-gcc 15.2.0 are generated from the `releases/gcc-15.2.0` tag. The oldest
`avr-g++` is used to check the headers, since older compilers are stricter.

For every set it performs these steps:

1. Download the libstdc++ sources of the GCC release from the
   [GCC mirror](https://github.com/gcc-mirror/gcc) (a sparse checkout of about
   150 MB, cached in `$GCC_SRC_CACHE`, by default `/tmp/avr-libstdcpp`).
2. Run the libstdc++ configure script with the newest `avr-g++` and avr-libc,
   install the headers and flatten the target directory `avr/bits` into
   `bits`. Copy the out-of-line parts of the containers from the same release
   and our own sources from [`tools/src`](../tools/src).
3. Remove what is not useful on AVR: the TR1 and TR2 extensions, the
   experimental technical specifications, parallel mode, the policy-based data
   structures, decimal floating-point and the sources of the `std` module.
4. Remove the C compatibility headers `<math.h>`, `<stdlib.h>`, `<complex.h>`,
   `<fenv.h>` and `<tgmath.h>`, which clash with avr-libc. They would also
   declare the overloads of `std::abs()` in the global namespace, which makes
   calls like `abs(x)` ambiguous in existing code. Instead, a minimal
   `<stdlib.h>` only removes the `abs()` macro of avr-libc, which breaks
   `<chrono>`.
5. Configure `bits/c++config.h` for avr-libc:
   - enable the C99 math functions and the `float_t` and `double_t` types.
     avr-libc lacks a few of them, so the configure checks fail, even though
     most of them exist. The missing functions are removed in step 8,
   - disable the special math functions, which are implemented in the removed
     TR1 headers,
   - for GCC 13 and 14.1, undefine `__STDCPP_FLOAT32_T__` to avoid an internal
     compiler error.
   - remove the `extern template` declarations of `std::string` in
     `bits/basic_string.tcc`, since libstdc++ instantiates it in the compiled
     library. The compiler then instantiates only the members that are used.
     The other `extern template` declarations are kept, since they are only
     used by I/O streams and locales, and removing them would add a static
     initializer for the locale facets to every translation unit.
6. Patch other headers:
   - remove `[[likely]]` after `if` statements if `avr-g++` is older than 9,
   - replace the C++23 decay-copy `auto(x)` if `avr-g++` is older than 12,
   - add the AVR fallback for `std::errc::value_too_large` from GCC 13 to
     older headers, since avr-libc has no `EOVERFLOW`,
   - undefine the float math macros of avr-libc < 2.1 in `<cmath>`,
   - define the floating-point classification macros `FP_NAN`, … and the
     `float_t` and `double_t` types in `<cmath>`, which avr-libc lacks.
7. Mark all headers as system headers with `#pragma GCC system_header`, since
   they are often included with `-I` instead of `-isystem`.
8. Compile all headers with `avr-g++` and comment out every
   `using ::name;` declaration whose name is not declared by avr-libc. The
   missing C99 math functions are replaced by calls to the compiler builtins,
   so that calls with `double` arguments are not ambiguous.
9. Compile all headers again with C++17 and the newest standard supported by
   the oldest `avr-g++` and fail on any error.

All modifications are marked with an `avr-libstdcpp` comment, so you can find
them with `grep -r avr-libstdcpp gcc15/include`. Every patch fails the
generation if it no longer applies, except the ones that are only required for
some GCC versions.


## Redirecting to the header sets

After all sets are generated into `gcc10`, `gcc12`, `gcc13` and `gcc15`, the
`include` and `src` directories are regenerated. For every file in any set,
they contain a file that redirects to the header set of the avr-gcc version:

```cpp
#pragma GCC system_header // avr-libstdcpp
#if __GNUC__ >= 14
#include "../gcc15/include/vector"
#elif __GNUC__ >= 13
#include "../gcc13/include/vector"
#elif __GNUC__ >= 11
#include "../gcc12/include/vector"
#elif __GNUC__ >= 8
#include "../gcc10/include/vector"
#else
#error "avr-libstdcpp requires avr-gcc 8 or newer"
#endif
```

The redirected header includes its dependencies via the include path again,
which redirect to the same set, so every compiler only uses the headers of its
own set with a single include path. If a set does not contain a file, for
example `<print>` in `gcc13`, the redirect fails with an `#error` for these
compilers. Therefore `__has_include(<print>)` is true for all compilers, check
the `__cpp_lib_print` macro of `<version>` instead or add the `include`
directory of the header set to the include path directly.

`<stdlib.h>` is not redirected, but copied, since it is the same in all sets
and uses `#include_next`, which only works for headers found via the include
path.


## Regenerating all header sets

`tools/generate.py` generates all header sets with the compilers from
Homebrew:

```sh
brew tap osx-cross/avr
brew install avr-gcc@8 avr-gcc@10 avr-gcc@11 avr-gcc@12 avr-gcc@13 avr-gcc@14 avr-gcc@15
tools/generate.py
```

On other systems, set the compilers with environment variables:

```sh
AVR_GXX_15=/opt/avr-gcc-15/bin/avr-g++ \
AVR_GXX_14=/opt/avr-gcc-14/bin/avr-g++ \
tools/generate.py 15:14
```

The output must be reproducible: the CI regenerates all sets with the newest
avr-gcc releases every month and fails if anything changed, for example when
Homebrew updates avr-gcc@15 to a new GCC release. Then regenerate the sets
locally, run the tests and commit the result.


## Library sources

Every header set contains the sources that are normally compiled into
libstdc++ and are required by the headers. They are copied from the same
GCC release as the headers:

| File                  | Content                                                   |
|-----------------------|-----------------------------------------------------------|
| `hashtable_c++0x.cc`  | The rehash policy of the unordered containers             |
| `hashtable-aux.h`     | The prime numbers for the bucket counts                   |
| `list.cc`             | The node algorithms of `std::list`                        |
| `tree.cc`             | The red-black tree of `std::map` and `std::set`           |
| `new_handler.cc`      | `std::set_new_handler()` and `std::nothrow`               |

The only change is that `hashtable-aux.cc` is renamed to `hashtable-aux.h`, so
that it is not compiled on its own. The sources in [`tools/src`](../tools/src)
are specific to AVR and maintained here. Change them there and regenerate all
sets:

| File                  | Content                                                   |
|-----------------------|-----------------------------------------------------------|
| `functexcept.cc`      | `std::__throw_*` functions, weak and calling `abort()`    |
| `hash_bytes.cc`       | `std::hash` for strings, using a 16-bit CRC               |
| `math.cc`             | Missing math functions of older avr-libc versions         |

## Adding a new GCC version

For example for GCC 16:

1. Install `avr-gcc@16`.
2. Check which avr-gcc versions can parse the new headers by generating them
   into `gcc16` with the oldest candidate: `tools/generate.py 16:15`
3. If the generation fails, the script prints the compiler errors. Adapt the
   patches in `tools/generate.py` so that they work for all versions.
4. Add the new set to `SETS` in `tools/generate.py`, the C++ standard to
   `avr-libstdcpp.mk`, [the compatibility table](compatibility.md) and the CI
   matrix.
5. Run the [integration tests](testing.md) with every compiler that uses the
   new set.
