#ifndef _INCLUDED_location_hash_algorithm_hpp
#define _INCLUDED_location_hash_algorithm_hpp

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
	 * Combines the hash of a single value with a seed value.
	 *
	 * @tparam T The type of the value to hash. Must be hashable by std::hash.
	 * @param seed The current seed value.
	 * @param value The value to hash.
	 * @return A combined hash value.
	 */
	template <typename T>
	size_t hash_combine(size_t seed, const T & value)
	{
		std::hash<T> hasher;
		// Combines the current seed with the hash of the value using bitwise operations
		//
		// Explanation:
		// 1. hasher(value): Computes the hash of the input value.
		// 2. seed << 6: Left shift the seed by 6 bits. This is equivalent to multiplying the seed by 64.
		// 3. seed >> 2: Right shift the seed by 2 bits. This is equivalent to dividing the seed by 4.
		// 4. 0x9e3779b9: A large constant (part of the golden ratio) used to distribute the hash values more uniformly.
		// 5. The final result is obtained by combining these values using bitwise XOR (^) and addition (+) operations.
		//
		// Why bitwise operations:
		// - Bitwise operations (<<, >>, ^) are generally faster than arithmetic operations (e.g., multiplication,
		// division)
		//   because they operate directly on the binary representation of the numbers.
		// - They help in spreading out the bits more uniformly, which reduces the chances of hash collisions.
		// - The combination of shifts and XOR operations helps in mixing the bits of the hash value and seed, leading
		// to a more
		//   evenly distributed hash result.
		return seed ^ (hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
	}

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
	 * Generates a hash from an array of values with precision.
	 * All input parameters must be of the same type, and this is enforced at compile time.
	 *
	 * @tparam Precision The precision value for quantization. Must be a power of two.
	 * @tparam CoordinateType The type of the values to hash. Must be an arithmetic type.
	 * @tparam Dimensions The number of dimensions (size of the array).
	 * @param coordinates The array of values to hash.
	 * @return A combined hash value.
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions>
	size_t generate_hash(const std::array<CoordinateType, Dimensions> & coordinates)
	{
		static_assert(std::is_arithmetic<CoordinateType>::value, "Only arithmetic types are supported");
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");

		size_t seed = 0;

		// Initial seed with quantized first value
		// Quantization ensures values are grouped into buckets defined by Precision
		for (const auto & value : coordinates) {
			seed = hash_combine(seed, quantize_value<CoordinateType, Precision>(value));
		}
		return seed;
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

	/**
	 * Generates all hash keys within a range defined by min and max coordinates.
	 *
	 * @tparam Precision The precision value for quantization. Must be a power of two.
	 * @tparam CoordinateType The type of the coordinates. Must be an arithmetic type.
	 * @tparam Dimensions The number of dimensions.
	 * @param min_coords The minimum coordinates for the range.
	 * @param max_coords The maximum coordinates for the range.
	 * @return A vector of hash keys for all coordinates within the range.
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions>
	std::vector<size_t> generate_all_hash_keys_within_range(const std::array<CoordinateType, Dimensions> & min_coords,
	                                                        const std::array<CoordinateType, Dimensions> & max_coords)
	{
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");

		std::vector<size_t>            hash_keys;
		std::array<size_t, Dimensions> steps;
		constexpr size_t               precision_shift = calculate_precision_shift<Precision>();

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

			size_t hash_key = generate_hash<Precision, CoordinateType, Dimensions>(current_coords);
			hash_keys.push_back(hash_key);

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

		return hash_keys;
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

	/***
	 * Generate all hash keys within a certain distance from a point.
	 *
	 * @tparam Precision The precision value for quantization. Must be a power of two.
	 * @tparam CoordinateType The type of the coordinates.
	 * @tparam Dimensions The number of dimensions.
	 * @param center The center point to calculate distance from.
	 * @param radius The distance from the center point.
	 * @return A vector of hash keys within the specified distance.
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions>
	std::vector<size_t> generate_all_hash_keys_within_distance(const std::array<CoordinateType, Dimensions> & center,
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

		return generate_all_hash_keys_within_range<Precision, CoordinateType, Dimensions>(lower_bounds, upper_bounds);
	}

} // namespace lochash

#endif //_INCLUDED_location_hash_algorithm_hpp
