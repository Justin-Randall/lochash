#ifndef _INCLUDED_location_hash_algorithm_hpp
#define _INCLUDED_location_hash_algorithm_hpp

#include "real_to_int.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <vector>

using ssize_t = std::make_signed<size_t>::type;

namespace lochash
{
	/**
	 * Quantizes a value based on the specified precision.
	 * This function uses bitwise operations for performance, avoiding division.
	 *
	 * @tparam T The type of the value to quantize. Must be an arithmetic type.
	 * @tparam Precision The precision value. Must be a power of two.
	 * @param value The value to quantize.
	 * @return The quantized value.
	 */
	template <typename T, size_t Precision, typename QuantizedCoordinateIntegerType = int64_t>
	constexpr QuantizedCoordinateIntegerType quantize_value(T value)
	{
		static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");

		// Using bitwise AND with ~(Precision - 1) to quantize the value
		// This is more performant than using division or modulo operations because
		// bitwise operations are generally faster and have lower latency in modern CPUs.
		// Division and modulo operations typically take multiple CPU cycles to complete,
		// whereas bitwise operations usually execute in a single cycle.
		//
		// TODO : the built-in std::round or casting to it may not be the fastest way
		//  to quantize the coordinates. Consider using a faster method. SSE or AVX allow
		//  for parallel operations on multiple values, not be as slow as built-in implementations
		//  and may deal with floating point state and precision which could be faster than the
		//  current method.
		QuantizedCoordinateIntegerType quantized_value = real_to_int<T, QuantizedCoordinateIntegerType>(value);
		quantized_value                                = quantized_value & ~(Precision - 1);
		// return static_cast<QuantizedCoordinateIntegerType>(value) & ~(Precision - 1);
		return quantized_value;
	}

	/**
	 * Calculates the shift value for the precision.
	 * This function is used to determine the number of bits to shift for quantization.
	 *
	 * It is constexpr correct and is evaluated at compile time.
	 *
	 * @tparam Precision The precision value. Must be a power of two.
	 * @return The shift value for the precision.
	 */
	template <size_t Precision>
	constexpr size_t calculate_precision_shift()
	{
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");
		size_t shift = 0;
		size_t value = Precision;
		while (value > 1) {
			value >>= 1;
			++shift;
		}
		return shift;
	}

	// Helper function to calculate squared difference between coordinates
	template <typename CoordinateType>
	CoordinateType squared_difference(CoordinateType a, CoordinateType b)
	{
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");
		return (a - b) * (a - b);
	}

	template <typename CoordinateType, size_t Dimensions>
	CoordinateType calculate_distance_squared(const std::array<CoordinateType, Dimensions> & point1,
	                                          const std::array<CoordinateType, Dimensions> & point2)
	{
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");
		CoordinateType distance_squared = 0;
		for (size_t i = 0; i < Dimensions; ++i) {
			distance_squared += squared_difference(point1[i], point2[i]);
		}
		return distance_squared;
	}
} // namespace lochash

#endif //_INCLUDED_location_hash_algorithm_hpp
