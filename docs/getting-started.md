# Getting started

Using avr-libstdcpp takes three steps:

1. Add the `include` directory to the include path.
2. Compile and link the `src` directory with your application.
3. Provide the few runtime functions that the C++ language needs.

The libstdc++ headers of one GCC version cannot be parsed by every other GCC
version, so there are four header sets (see [compatibility](compatibility.md)).
You do not need to choose one: The headers and sources in `include` and `src`
redirect to the set of your avr-gcc version. The C++ standards differ however:

| avr-gcc  | Header set | Newest C++ standard |
|----------|------------|---------------------|
| 8, 9     | `gcc10`    | C++17               |
| 10       | `gcc10`    | C++20               |
| 11, 12   | `gcc12`    | C++23               |
| 13       | `gcc13`    | C++23               |
| 14, 15   | `gcc15`    | C++26               |

You can check your version with `avr-g++ -dumpversion`.

Including a header that is not available for your compiler, for example
`<print>` with avr-gcc 13, fails with an `#error`. So `__has_include()` is
always true, use the `__cpp_lib_*` macros of `<version>` instead, or add the
`include` directory of the header set directly, for example `gcc13/include`.


## 1. and 2. Compile with the library

```sh
LIB=path/to/avr-libstdcpp
avr-g++ -std=c++23 -Os -mmcu=atmega328p -fno-exceptions -fno-rtti -fno-threadsafe-statics \
        -ffunction-sections -fdata-sections -I$LIB/include -c main.cpp $LIB/src/*.cc
avr-g++ -mmcu=atmega328p -Wl,--gc-sections *.o -o main.elf -lm
```

The flags in detail:

- `-I$LIB/include`: The headers mark themselves as system headers, so you will
  not see warnings from them. Do not use `-isystem` with avr-gcc 8: it wraps
  system include directories in an implicit `extern "C"`, which breaks all
  templates.
- `-fno-exceptions -fno-rtti`: Exceptions and run-time type information are
  not supported. Errors that would throw an exception call `abort()` instead
  (see [runtime](#3-provide-the-runtime)).
- `-fno-threadsafe-statics`: There are no threads, so the guards for static
  local variables are not needed. Alternatively compile `runtime/cxxabi.cpp`.
- `-ffunction-sections -fdata-sections -Wl,--gc-sections`: The library sources
  contain many functions that your application may not use. This removes them,
  for example a small program shrinks from 10kB to 300B.
- `-lm`: Several templates call math functions, for example the rehash policy
  of the unordered containers calls `ceil()`, so always link with the avr-libc
  math library.

You may also need these flags depending on your compiler:

- `-Wno-volatile`: C++20 deprecated compound assignments to volatile variables,
  such as `PORTB |= 1`, which C++23 allowed again. avr-gcc 12 warns about them
  in C++20 and C++23 mode.
- `--param=min-pagesize=0`: avr-gcc 12 and 13 warn about every register access with
  `-Warray-bounds` ([GCC bug 105523](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523)).
- `-fcoroutines`: avr-gcc 10 requires it to use `<coroutine>`.


### With Make

The `avr-libstdcpp.mk` file provides the paths and the newest C++ standard
for your compiler:

```make
AVR_CXX := avr-g++
include path/to/avr-libstdcpp/avr-libstdcpp.mk

CXXFLAGS += -std=$(AVR_LIBSTDCPP_STD) -I$(AVR_LIBSTDCPP_INCLUDE)
SOURCES += $(AVR_LIBSTDCPP_SOURCES)
```

It defines these variables:

| Variable                | Content                                            |
|-------------------------|----------------------------------------------------|
| `AVR_LIBSTDCPP_GCC`     | The major version of `$(AVR_CXX)`, e.g. `15`       |
| `AVR_LIBSTDCPP_STD`     | The newest supported C++ standard, e.g. `c++26`    |
| `AVR_LIBSTDCPP_DIR`     | The path to avr-libstdcpp                          |
| `AVR_LIBSTDCPP_INCLUDE` | The include path                                   |
| `AVR_LIBSTDCPP_SOURCES` | The library sources that must be compiled          |

See [`examples/common.mk`](../examples/common.mk) for a complete example.


### With CMake

```cmake
set(AVR_LIBSTDCPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/avr-libstdcpp)
file(GLOB AVR_LIBSTDCPP_SOURCES ${AVR_LIBSTDCPP_DIR}/src/*.cc)
target_sources(firmware PRIVATE ${AVR_LIBSTDCPP_SOURCES})
target_include_directories(firmware PRIVATE ${AVR_LIBSTDCPP_DIR}/include)
target_compile_options(firmware PRIVATE -fno-exceptions -fno-rtti -fno-threadsafe-statics
                                        -ffunction-sections -fdata-sections)
target_link_options(firmware PRIVATE -Wl,--gc-sections)
target_link_libraries(firmware PRIVATE m)
```


### With Microchip Studio (Atmel Studio)

Add the `include` directory to the include paths of the project and add the
files in the `src` directory to the project. The avr-gcc
shipped with Microchip Studio is too old, so you also have to install a newer
avr-gcc and configure the project to use it. See
[issue 17](https://github.com/modm-io/avr-libstdcpp/issues/17#issuecomment-1098241768)
for details.


## 3. Provide the runtime

The C++ language relies on a few functions that avr-libc does not provide.
The [`runtime`](../runtime) directory contains simple implementations that you
can use or replace with your own:

| File                                  | Provides                                  | Needed for                    |
|---------------------------------------|-------------------------------------------|-------------------------------|
| [`new.cpp`](../runtime/new.cpp)       | `operator new` and `delete` using `malloc` | containers, `std::function`, `std::make_unique` |
| [`cxxabi.cpp`](../runtime/cxxabi.cpp) | `__cxa_pure_virtual`, static guards       | pure virtual functions, statics without `-fno-threadsafe-statics` |
| [`atomic.cpp`](../runtime/atomic.cpp) | `__atomic_*` functions                    | `std::atomic`                 |

avr-gcc implements every `std::atomic` operation as a call to an `__atomic_*`
library function, since AVR has no atomic instructions.
`runtime/atomic.cpp` makes them atomic by disabling interrupts.
Since AVR does not nest interrupts, an interrupt handler does not need atomic
read-modify-write operations: a relaxed `load()` and `store()` is sufficient,
and compiles to plain instructions for single bytes. See the
[interrupt_counter](../examples/interrupt_counter/main.cpp) example.

The clocks of `<chrono>` depend on your hardware, so you have to implement
their `now()` functions yourself. For example with a millisecond counter:

```cpp
std::chrono::steady_clock::time_point
std::chrono::steady_clock::now() noexcept
{
    return time_point{std::chrono::milliseconds{millis()}};
}
```

See [`examples/common/clock.cpp`](../examples/common/clock.cpp) for a
complete implementation with Timer0. `std::chrono::system_clock` and its alias
`std::chrono::high_resolution_clock` work the same way.

The library sources also define the functions that report errors, such as
`std::__throw_out_of_range()` called by `std::vector::at()`. They are weak
functions that call `abort()`, so you can override them to report the error:

```cpp
namespace std
{
void __throw_out_of_range(const char* what)
{
    printf("out of range: %s\n", what);
    abort();
}
}
```


## Next steps

- The [examples](../examples) show real-world uses on an Arduino Uno.
- The [limitations](limitations.md) explain what is not supported.
- The [guide](embedded-guide.md) explains which parts of the library are most
  useful on small microcontrollers.
