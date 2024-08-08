#ifndef _INCLUDED_location_hash_algorithm_hpp
#define _INCLUDED_location_hash_algorithm_hpp

#include "location_hash_quantized_coordinate.hpp"
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
	template <typename T, size_t Precision>
	constexpr ssize_t quantize_value(T value)
	{
		static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");

		// Using bitwise AND with ~(Precision - 1) to quantize the value
		// This is more performant than using division or modulo operations because
		// bitwise operations are generally faster and have lower latency in modern CPUs.
		// Division and modulo operations typically take multiple CPU cycles to complete,
		// whereas bitwise operations usually execute in a single cycle.
		return static_cast<size_t>(value) & ~(Precision - 1);
	}

	/**
	 * Quantizes an array of coordinates based on the specified precision.
	 *
	 * @tparam Precision The precision value for quantization. Must be a power of two.
	 * @tparam CoordinateType The type of the coordinates. Must be an arithmetic type.
	 * @tparam Dimensions The number of dimensions.
	 * @param coordinates The array of coordinates to quantize.
	 * @return A quantized array of coordinates.
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions>
	std::array<size_t, Dimensions> quantize_coordinates(const std::array<CoordinateType, Dimensions> & coordinates)
	{
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type");

		std::array<size_t, Dimensions> quantized_coords;
		for (size_t i = 0; i < Dimensions; ++i) {
			quantized_coords[i] = quantize_value<CoordinateType, Precision>(coordinates[i]);
		}
		return quantized_coords;
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

	/**
	 * @brief Quantizes the lower and upper bounds specficied by min_coords and max_coords respectively
	 *   to return a vector of all quantized coordinates within the specified range. These may be used
	 *   as keys to search the location hash for non-empty buckets.
	 *
	 *   For example, coordinates from (-24.4f, -15.0f) to (24.4f, 15.0f) with a precision of 4 will
	 *   return 4^2 = 16 distinct quantized coordinates -- (-24, 12), (-24, 8) ... and so on until(24, 12).
	 *
	 *   The map may contain zero (not found) or more entries for each quantized coordinate.
	 *
	 * @tparam Precision
	 * @tparam CoordinateType
	 * @tparam Dimensions
	 * @param min_coords
	 * @param max_coords
	 * @return std::vector<QuantizedCoordinate<Precision, CoordinateType, Dimensions>>
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions>
	std::vector<QuantizedCoordinate<Precision, CoordinateType, Dimensions>>
	generate_all_quantized_coordinates_within_range(const std::array<CoordinateType, Dimensions> & min_coords,
	                                                const std::array<CoordinateType, Dimensions> & max_coords)
	{
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");

		std::vector<QuantizedCoordinate<Precision, CoordinateType, Dimensions>> quantized_coords;
		std::array<size_t, Dimensions>                                          steps;
		constexpr size_t precision_shift = calculate_precision_shift<Precision>();

		// Calculate the number of steps required for each dimension
		for (size_t i = 0; i < Dimensions; ++i) {
			steps[i] = ((quantize_value<CoordinateType, Precision>(max_coords[i]) -
			             quantize_value<CoordinateType, Precision>(min_coords[i])) >>
			            precision_shift) +
			           1;
		}

		std::array<size_t, Dimensions> indices = {0};
		bool                           done    = false;

		while (!done) {
			std::array<CoordinateType, Dimensions> current_coords;
			for (size_t i = 0; i < Dimensions; ++i) {
				current_coords[i] = min_coords[i] + static_cast<CoordinateType>(indices[i] << precision_shift);
			}

			quantized_coords.emplace_back(current_coords);

			// Increment the indices array to generate the next coordinate in the range.
			for (size_t i = 0; i < Dimensions; ++i) {
				if (++indices[i] < steps[i]) {
					break;
				}
				indices[i] = 0;
				if (i == Dimensions - 1) {
					done = true;
				}
			}
		}

		return quantized_coords;
	}

	/**
	 * @brief Calculates lower and upper bounds of a bounding box centered at the specified center, then
	 *  passes the calculation to generate_all_quantized_coordinates_within_range to return a vector of
	 *  all quantized coordinates within the specified distance. These may be used as keys to search the
	 *  location hash for non-empty buckets.
	 *
	 * @tparam Precision
	 * @tparam CoordinateType
	 * @tparam Dimensions
	 * @param center
	 * @param radius
	 * @return std::vector<QuantizedCoordinate<Precision, CoordinateType, Dimensions>>
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions>
	std::vector<QuantizedCoordinate<Precision, CoordinateType, Dimensions>>
	generate_all_quantized_coordinates_within_distance(const std::array<CoordinateType, Dimensions> & center,
	                                                   CoordinateType                                 radius)
	{
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

		std::array<CoordinateType, Dimensions> lower_bounds;
		std::array<CoordinateType, Dimensions> upper_bounds;

		for (size_t i = 0; i < Dimensions; ++i) {
			lower_bounds[i] = center[i] - radius;
			upper_bounds[i] = center[i] + radius;
		}
		return generate_all_quantized_coordinates_within_range<Precision, CoordinateType, Dimensions>(lower_bounds,
		                                                                                              upper_bounds);
	}
} // namespace lochash

#endif //_INCLUDED_location_hash_algorithm_hpp
