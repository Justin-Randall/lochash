#ifndef _INCLUDED_location_hash_quantized_coordinate_hpp
#define _INCLUDED_location_hash_quantized_coordinate_hpp

// --------------------------------------------------------------------------------------------
// Disable some warnings that are spurious in this context. The standard
// actually specifies that unknown pragmas should be ignored, but some
// compilers still complain about them. Support -Wall and -Werror anyway
// by without warnings for unknown pragmas.
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif
// --------------------------------------------------------------------------------------------

#include <array>
#include <functional>
#include <vector>

// Check if we're on an x86-64 platform with SSE2 support
#if defined(_M_X64) || defined(__x86_64__) || defined(__AVX2__) || defined(__SSE4_2__)
#include <immintrin.h>
#define USE_SIMD 1
#else
#define USE_SIMD 0
#endif

#include "location_hash_algorithm.hpp"

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
	template <size_t Precision, typename CoordinateType, size_t Dimensions,
	          typename QuantizedCoordinateIntegerType = int64_t>
	struct QuantizedCoordinate {

		/**
		 * @brief Construct a new Quantized Coordinate object and provide implicit conversion for CoordinateType arrays.
		 *
		 * @param coordinates
		 */
		QuantizedCoordinate(const std::array<CoordinateType, Dimensions> & coordinates)
		{
			for (size_t i = 0; i < Dimensions; ++i) {
				quantized_[i] =
				    quantize_value<CoordinateType, Precision, QuantizedCoordinateIntegerType>(coordinates[i]);
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
#if USE_SIMD // Really annoying that compilers do not already emit this code, but also understandable
#pragma warning(push)
#pragma warning(disable : 4800) // yes, I know what I am doing here
			if constexpr (Dimensions == 1) {
				return quantized_[0] == other.quantized_[0];
			} else if constexpr (Dimensions == 2) {
				if constexpr (sizeof(QuantizedCoordinateIntegerType) == 8) {
					__m128i val1 = _mm_set_epi64x(quantized_[1], quantized_[0]);
					__m128i val2 = _mm_set_epi64x(other.quantized_[1], other.quantized_[0]);
					return _mm_testc_si128(_mm_cmpeq_epi64(val1, val2), _mm_set1_epi64x(-1));
				} else if constexpr (sizeof(QuantizedCoordinateIntegerType) == 4) {
					__m128i val1 = _mm_set_epi32(quantized_[1], quantized_[0], 0, 0);
					__m128i val2 = _mm_set_epi32(other.quantized_[1], other.quantized_[0], 0, 0);
					return _mm_testc_si128(_mm_cmpeq_epi32(val1, val2), _mm_set1_epi32(-1));
				} else {
					for (size_t i = 0; i < Dimensions; ++i) {
						if (quantized_[i] != other.quantized_[i]) {
							return false;
						}
					}
					return true;
				}
			} else if constexpr (Dimensions == 3) {
				if constexpr (sizeof(QuantizedCoordinateIntegerType) == 8) {
					__m256i val1 = _mm256_set_epi64x(0, quantized_[2], quantized_[1], quantized_[0]);
					__m256i val2 = _mm256_set_epi64x(0, other.quantized_[2], other.quantized_[1], other.quantized_[0]);
					return _mm256_testc_si256(_mm256_cmpeq_epi64(val1, val2), _mm256_set1_epi64x(-1));
				} else if constexpr (sizeof(QuantizedCoordinateIntegerType) == 4) {
					__m128i val1 = _mm_set_epi32(0, quantized_[2], quantized_[1], quantized_[0]);
					__m128i val2 = _mm_set_epi32(0, other.quantized_[2], other.quantized_[1], other.quantized_[0]);
					return _mm_testc_si128(_mm_cmpeq_epi32(val1, val2), _mm_set1_epi32(-1));
				} else {
					for (size_t i = 0; i < Dimensions; ++i) {
						if (quantized_[i] != other.quantized_[i]) {
							return false;
						}
					}
					return true;
				}
			} else if constexpr (Dimensions == 4) {
				if constexpr (sizeof(QuantizedCoordinateIntegerType) == 8) {
					__m256i val1 = _mm256_set_epi64x(quantized_[3], quantized_[2], quantized_[1], quantized_[0]);
					__m256i val2 = _mm256_set_epi64x(other.quantized_[3], other.quantized_[2], other.quantized_[1],
					                                 other.quantized_[0]);
					return _mm256_testc_si256(_mm256_cmpeq_epi64(val1, val2), _mm256_set1_epi64x(-1));
				} else if constexpr (sizeof(QuantizedCoordinateIntegerType) == 4) {
					__m128i val1 = _mm_set_epi32(quantized_[3], quantized_[2], quantized_[1], quantized_[0]);
					__m128i val2 = _mm_set_epi32(other.quantized_[3], other.quantized_[2], other.quantized_[1],
					                             other.quantized_[0]);
					return _mm_testc_si128(_mm_cmpeq_epi32(val1, val2), _mm_set1_epi32(-1));
				} else {
					for (size_t i = 0; i < Dimensions; ++i) {
						if (quantized_[i] != other.quantized_[i]) {
							return false;
						}
					}
					return true;
				}
			} else {
				for (size_t i = 0; i < Dimensions; ++i) {
					if (quantized_[i] != other.quantized_[i]) {
						return false;
					}
				}
				return true;
			}
#pragma warning(pop)
#else
			// Old way that works, but is slower
			for (size_t i = 0; i < Dimensions; ++i) {
				if (quantized_[i] != other.quantized_[i]) {
					return false;
				}
			}
			return true;
#endif
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
	template <size_t Precision, typename CoordinateType, size_t Dimensions, typename QuantizedCoordinateIntegerType>
	struct hash<lochash::QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>> {
		size_t operator()(const lochash::QuantizedCoordinate<Precision, CoordinateType, Dimensions,
		                                                     QuantizedCoordinateIntegerType> & qc) const
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
	template <size_t Precision, typename CoordinateType, size_t Dimensions,
	          typename QuantizedCoordinateIntegerType = int64_t>
	std::vector<QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>>
	generate_all_quantized_coordinates_within_range(const std::array<CoordinateType, Dimensions> & min_coords,
	                                                const std::array<CoordinateType, Dimensions> & max_coords)
	{
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");

		std::vector<QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>>
		                               quantized_coords;
		std::array<size_t, Dimensions> steps;
		constexpr size_t               precision_shift = calculate_precision_shift<Precision>();

		// Calculate the number of steps required for each dimension
		size_t total_steps = 1;
		for (size_t i = 0; i < Dimensions; ++i) {
			steps[i] = ((quantize_value<CoordinateType, Precision, QuantizedCoordinateIntegerType>(max_coords[i]) -
			             quantize_value<CoordinateType, Precision, QuantizedCoordinateIntegerType>(min_coords[i])) >>
			            precision_shift) +
			           1;
			total_steps *= steps[i];
		}

		// reserve space in the vector to avoid reallocations
		quantized_coords.reserve(total_steps);

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
	template <size_t Precision, typename CoordinateType, size_t Dimensions,
	          typename QuantizedCoordinateIntegerType = int64_t>
	std::vector<QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>>
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
		return generate_all_quantized_coordinates_within_range<Precision, CoordinateType, Dimensions,
		                                                       QuantizedCoordinateIntegerType>(lower_bounds,
		                                                                                       upper_bounds);
	}
} // namespace lochash

#endif //_INCLUDED_location_hash_quantized_coordinate_hpp
