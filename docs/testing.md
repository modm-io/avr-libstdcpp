# Testing

## Integration tests

The [integration tests](../tests) exercise a representative part of the
library: containers, strings, algorithms, vocabulary types, math, random
numbers, atomics, chrono, and the C++20 to C++26 features. Each test checks
its results at runtime.

The tests are built for an ATmega2560 with every C++ standard from C++17 to
the newest one supported by the compiler, and run in [simavr](https://github.com/buserror/simavr).
They report their results via USART0:

```
$ cd tests
$ make AVR_CXX=avr-g++
gcc15 c++17 containers: PASSED 22/22
gcc15 c++17 utilities: PASSED 28/28
gcc15 c++17 modern: PASSED 2/2
gcc15 c++20 containers: PASSED 22/22
gcc15 c++20 utilities: PASSED 28/28
gcc15 c++20 modern: PASSED 13/13
gcc15 c++23 containers: PASSED 22/22
gcc15 c++23 utilities: PASSED 28/28
gcc15 c++23 modern: PASSED 15/15
gcc15 c++26 containers: PASSED 22/22
gcc15 c++26 utilities: PASSED 28/28
gcc15 c++26 modern: PASSED 15/15
```

The C++20 to C++26 features in `modern.cpp` are guarded by their feature-test
macros, so the number of checks depends on the compiler.

Use `make build` to only compile and link, for example without simavr.
To see which check hangs or crashes, compile with `-DTEST_VERBOSE`, which
prints every checked line.

The tests use the [runtime](../runtime) and deterministic clocks from
[`clock.cpp`](../tests/clock.cpp), which advance by 1ms on every call.


## Examples

The [examples](../examples) target an Arduino Uno. They are built by the CI,
but not run, since they depend on hardware. You can run them in simavr:

```
$ cd examples/blink
$ make sim
uptime 00:00:00
uptime 00:00:01
```

Examples that require a newer C++ standard than the compiler supports are
skipped.


## Continuous integration

The [CI](../.github/workflows/ci.yml) runs these jobs:

- **Tests**: builds and runs the integration tests and builds all examples with
  every supported avr-gcc version from Homebrew, avr-gcc 8 to 15.
- **Generate**: installs the newest toolchains of each GCC version,
  regenerates all header sets and fails if the result differs from the
  committed files. It also fails if Homebrew provides a newer GCC version than
  the newest header set. This job also runs monthly, so that new toolchain
  releases are noticed.
- **Linux**: builds the tests and examples with the avr-gcc of the
  [modm build image](https://github.com/modm-ext/docker-modm-build).
- **Distributions**: builds the tests and examples with the avr-gcc packages
  of Debian trixie (also used by Ubuntu 26.04) and Fedora. The tests only run
  on Debian, since Fedora does not package simavr.
- **Windows**: builds the tests and examples with the Windows builds of
  avr-gcc 8.3 to 15.2 (except 10.1) by [Zak Kemble](https://github.com/ZakKemble/avr-gcc-build).
  These are older point releases than the Homebrew builds. There is no simavr
  for Windows, so the tests are not run.
- **Keep-alive**: re-enables the workflow, since GitHub disables scheduled
  workflows after 60 days without activity in the repository.
