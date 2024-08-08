#ifndef _INCLUDED_location_hash_quantized_coordinate_hpp
#define _INCLUDED_location_hash_quantized_coordinate_hpp

#include "location_hash_types.hpp"
#include <array>
#include <functional>
#include <vector>

namespace lochash
{
	/**
	 * @brief A QuantizedCoordinate is a coordinate that has been quantized to a specific precision. It is a fixed,
	 *  c-style array of QuantizedCoordinateIntegerType (defaults to size_t). So, for example, a QuantizedCoordinate<16,
	 * float, 3> would be a 3D coordinate quantized to a stride of 16 (must be a power of 2). Internally, it uses some
	 * bitwise operations to quantize the values, which is faster than using division or modulo operations.
	 *
	 * @tparam Precision
	 * @tparam CoordinateType
	 * @tparam Dimensions
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions>
	struct QuantizedCoordinate {

		/**
		 * @brief Construct a new Quantized Coordinate object and provide implicit conversion for CoordinateType arrays.
		 *
		 * @param coordinates
		 */
		QuantizedCoordinate(const std::array<CoordinateType, Dimensions> & coordinates)
		{
			for (size_t i = 0; i < Dimensions; ++i) {
				// TODO : the built-in std::round or casting to it may not be the fastest way
				//  to quantize the coordinates. Consider using a faster method. SSE or AVX allow
				//  for parallel operations on multiple values, not be as slow as built-in implementations
				//  and may deal with floating point state and precision which could be faster than the
				//  current method.
				quantized_[i] = quantize_value<CoordinateType, Precision>(coordinates[i]);
			}
		}

		/**
		 * @brief Equality comparison operator to support map keys.
		 *
		 * @param other
		 * @return true
		 * @return false
		 */
		bool operator==(const QuantizedCoordinate & other) const
		{
			for (size_t i = 0; i < Dimensions; ++i) {
				if (quantized_[i] != other.quantized_[i]) {
					return false;
				}
			}
			return true;
		}

		/**
		 * @brief Comparison operator to support map keys.
		 *
		 * @param other
		 * @return true
		 * @return false
		 */
		bool operator<(const QuantizedCoordinate & other) const
		{
			for (size_t i = 0; i < Dimensions; ++i) {
				if (quantized_[i] < other.quantized_[i]) {
					return true;
				} else if (quantized_[i] > other.quantized_[i]) {
					return false;
				}
			}
			return false;
		}

		std::array<QuantizedCoordinateIntegerType, Dimensions> quantized_;
	};
} // namespace lochash

namespace std
{
	template <size_t Precision, typename CoordinateType, size_t Dimensions>
	struct hash<lochash::QuantizedCoordinate<Precision, CoordinateType, Dimensions>> {
		size_t operator()(const lochash::QuantizedCoordinate<Precision, CoordinateType, Dimensions> & qc) const
		{
			size_t seed = 0;
			for (size_t i = 0; i < Dimensions; ++i) {
				seed ^= qc.quantized_[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};
} // namespace std

namespace lochash
{
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

#endif //_INCLUDED_location_hash_quantized_coordinate_hpp
