# Guide: the C++ standard library on tiny bare-metal systems

This guide explains which parts of the standard library are particularly useful
on small microcontrollers and which require more care.

## Very helpful go-to libraries

In general the C++ standard library is intended
to be written and implemented in a resource-sensitive
fashion. This includes efforts to save on both
memory as well as run-time.
In fact, C++ standard library functions and algorithms
have, in general, been specifically written and tuned by
the library authors with efficiency aspects in mind.
In particular, library components compile reliably and quickly
and also lend themselves well to compiler optimization.

Some library components, however, are particularly well-suited
for bare-metal microcontroller programming.
These can be exceptionally helpful when used properly and sensibly
in tiny bare-metal microcontroller environments.

A subjective list of these the libraries/headers
and their main uses includes, but is not limited to,:

- `<array>` for containers having known, fixed size.
- `<algorithm>` for standard algorithms such as sorting, minimax, sequential operations, etc.
- `<cmath>` for projects requiring floating-point mathematical functions such as `std::sin()`, `std::exp()`, `std::frexp()` and many more. Remember to link with `-lm` and to compile the library sources, see [getting started](getting-started.md).
- `<cstdint>` which defines integral types having specified widths residing within `namespace std` like `std::uint8_t`.
- `<limits>` offering compile-time query of numeric limits of built-in types.
- `<numeric>` featuring a collection of useful numeric algorithms such as `std::accumulate()`, etc.
- `<type_traits>` for compile-time decisions based on types.

With these libraries alone, the entire project can benefit
from a great deal of the standard library's power without compromising
in any way on performance or sleek memory footprint.
This is because these libaries are typically lean, fast and require no additional storage.

The following non-trivial, real-world example, for instance,
wraps instances of an overly-simplified LED class abstraction
as object-references in an `std::array`.
Once stored, the application exercises the LED's toggle
function in an algorithmic loop with `toggle()`-method call
expressed via lambda function.

```cpp
#include <algorithm>
#include <array>
#include <type_traits>

class led
{
public:
  led() = default;

  auto toggle() -> void { }
};

led led0;
led led1;
led led2;

using led_ref_type = std::reference_wrapper<led>;

std::array<led_ref_type, 3U> led_refs =
{
  led0,
  led1,
  led2
};

int main()
{
  for(;;)
  {
    std::for_each(led_refs.begin(),
                  led_refs.end(),
                  [](led& lr) { lr.toggle(); });
  }
}
```

This nifty little example is terse, expressive and powerful.
It makes use of parts of `<algorithm>`, `<array>`  and `<type_traits>`
to greatly simplify the programming within a non-trivial
microcontroller situation.

This example is key because it combines the domains
of object-oriented programming with the templated
algorithms and wrappers of the STL to assist
in our microcontroller world.

## Libraries requiring more design considerations

Some C++ library and STL artifacts, however,
require more careful design considerations
regarding memory allocation and management.

Consider, for instance, `std::vector` from the `<vector>` library.
Vector creates a flexibly-sized array-like collection
of items of any kind, depending on the template parameter.

For instance:

```cpp
#include <vector>

// A vector of 3 integers.
std::vector<int> v { 1, 2, 3 };
```

See also the [command_shell](../examples/command_shell/main.cpp) example,
which uses `std::vector` and `std::map` with the default allocator.

This vector requires storage for three integers which,
on the `avr-gcc` platform is 6 bytes. The storage is managed
through vector's _second_, less well-known template parameter.
In other words,

```cpp
namespace std {

// Forward declaration of the vector template class.
template<typename T,
         typename AllocatorType = std::allocator<T>>
class vector;

}
```

Using containers requires memory allocation with a so-called allocator.
If none is specified, as in our code snippet, the default allocator
from namespace `std` for the templated type `T` of the vector is
automatically selected.

Good embeddable self-written custom allocators are essential for
using such containers so that memory could be managed with
a self-written memory pool, an off-chip memory device, etc.
A common selection is a pool of static memory creating a so-called
_ring_ _allocator_. This is an intermetiate/advanced topic
which will refine STL use _on_ _the_ _metal_ and
also allow for flexible template use in these resource-sensitive
realms.

## C++20 `constexpr` support

The following is a rather advanced, highly useful topic.
When using C++20, `constexpr` construction, assignment and evaluation
of various algorithms can and often will be generally
compile-time constant (i.e, via consistent use of C++20 `constexpr`-ness).

As a result of this, STL algorithms that use compile-time constant inputs are, in fact,
evaluated at compile time in C++20. This lets us perform a strong,
purposeful shift of algorithmic complexity _to_-_the_-_left_.
In other words, we shift algorithmic complexity _into_ the compile-time
stage of code development and _away from_ the precious RAM-ROM-space/cycles
of the compiled running code.

In the following code, for instance, we revisit the `std::array`/`<numeric>`
example from above. The variation below exhibits complete compile-time
evaluation of the algorithmic result.

To take the deep dive in this topic, follow all the useful compile-time
preprocessor symbols such as
`__cpp_lib_constexpr_algorithms`, `__cpp_lib_constexpr_numeric`, and many more
in [feature testing](https://en.cppreference.com/w/cpp/feature_test).

```cpp
#include <array>
#include <numeric>

#if (defined(__cpp_lib_constexpr_numeric) && (__cpp_lib_constexpr_numeric>=201911L))
#define MODM_CONSTEXPR constexpr
#define MODM_CONSTEXPR_NUMERIC_IS_CONSTEXPR 1
#else
#define MODM_CONSTEXPR
#define MODM_CONSTEXPR_NUMERIC_IS_CONSTEXPR 0
#endif

MODM_CONSTEXPR std::array<int, 3U> a { 1, 2, 3 };

int main()
{
  // 6
  auto MODM_CONSTEXPR sum = std::accumulate(a.cbegin(), a.cend(), 0);

  #if (MODM_CONSTEXPR_NUMERIC_IS_CONSTEXPR == 1)
  static_assert(sum == 6, "Error: Unexpected std::accumulate result!");
  #endif

  return (sum == 6 ? 0 : -1);
}
```

See also the [lookup_table](../examples/lookup_table/main.cpp) example,
which computes a complete sine table at compile time.

