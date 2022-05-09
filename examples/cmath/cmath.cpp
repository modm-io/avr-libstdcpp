/*
 * Copyright (c) 2022, Christopher Kormanyos
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <cmath>
#include <cstdint>
#include <limits>

namespace local {

namespace detail {

template<typename RealValueType, typename RealFunctionType>
auto integral
(
	const RealValueType& a,
	const RealValueType& b,
	const RealValueType& tol,
	RealFunctionType real_function
) noexcept -> RealValueType
{
	using real_value_type = RealValueType;

	std::uint_fast32_t n2(1);

	real_value_type step = ((b - a) / 2U);
	real_value_type result = (real_function(a) + real_function(b)) * step;

	const std::uint_fast8_t k_max = UINT8_C(32);

	for(std::uint_fast8_t k = UINT8_C(0); k < k_max; ++k)
	{
		real_value_type sum(0);

		for(std::uint_fast32_t j(0U); j < n2; ++j)
		{
			const std::uint_fast32_t two_j_plus_one = (j * UINT32_C(2)) + UINT32_C(1);

			sum += real_function(a + real_value_type(step * real_value_type(two_j_plus_one)));
		}

		const real_value_type tmp = result;

		result = (result / 2U) + (step * sum);

		using std::fabs;

		const real_value_type ratio = fabs(tmp / result);

		const real_value_type delta = fabs(ratio - 1U);

		if((k > UINT8_C(1)) && (delta < tol))
		{
			break;
		}

		n2 *= 2U;

		step /= 2U;
	}

	return result;
}

template<typename FloatingPointType>
auto is_close_fraction
(
	const FloatingPointType a,
	const FloatingPointType b,
	const FloatingPointType tol = FloatingPointType(std::numeric_limits<FloatingPointType>::epsilon() * FloatingPointType(100))
) noexcept -> bool
{
	using floating_point_type = FloatingPointType;

	using std::fabs;

	const floating_point_type ratio     = fabs(floating_point_type((floating_point_type(1) * a) / b));

	const floating_point_type closeness = fabs(floating_point_type(1 - ratio));

	return (closeness < tol);
}

// N[Pi, 51]
// 3.14159265358979323846264338327950288419716939937511
template<typename FloatingPointType> constexpr  FloatingPointType pi_v;
template<>                           constexpr float              pi_v<float>       = 3.14159265358979323846264338327950288419716939937511F;
template<>                           constexpr double             pi_v<double>      = 3.14159265358979323846264338327950288419716939937511;
template<>                           constexpr long double        pi_v<long double> = 3.14159265358979323846264338327950288419716939937511L;

} // namespace detail

template<typename FloatingPointType>
auto cyl_bessel_j(const std::uint_fast8_t n, const FloatingPointType& x) noexcept -> FloatingPointType
{
	using floating_point_type = FloatingPointType;

	constexpr floating_point_type epsilon = std::numeric_limits<floating_point_type>::epsilon();

	using std::cos;
	using std::sin;
	using std::sqrt;

	const auto tol = sqrt(epsilon);

	const auto integration_result =
	detail::integral
	(
		static_cast<floating_point_type>(0),
		detail::pi_v<floating_point_type>,
		tol,
		[&x, &n](const floating_point_type& t) noexcept -> floating_point_type
		{
			return cos(x * sin(t) - (t * static_cast<floating_point_type>(n)));
		});

	const auto jn = static_cast<floating_point_type>(integration_result / detail::pi_v<floating_point_type>);

	return jn;
}

} // namespace local

auto main() -> int
{
	using my_float_type = long double;

	static_assert((std::numeric_limits<my_float_type>::digits >= 24), "Error: Incorrect my_float_type type definition");

	constexpr my_float_type my_tol =
		static_cast<my_float_type>
		(
			std::numeric_limits<my_float_type>::epsilon() * static_cast<my_float_type>(100.0L)
		);

	// Compute y = cyl_bessel_j(2, 1.23) = 0.16636938378681407351267852431513159437103348245333
	// N[BesselJ[2, 123/100], 50]
	const my_float_type j2 = local::cyl_bessel_j(UINT8_C(2), static_cast<my_float_type>(1.23L));

	const bool result_is_ok =
		local::detail::is_close_fraction
		(
			static_cast<my_float_type>(0.1663693837868140735126785243L),
			j2,
			my_tol
		);

	return (result_is_ok ? 0 : -1);
}
