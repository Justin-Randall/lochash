#ifndef _INCLUDED_location_hash_quantized_coordinate_hpp
#define _INCLUDED_location_hash_quantized_coordinate_hpp

#include "location_hash_types.hpp"
#include <array>
#include <functional>

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

#endif //_INCLUDED_location_hash_quantized_coordinate_hpp
