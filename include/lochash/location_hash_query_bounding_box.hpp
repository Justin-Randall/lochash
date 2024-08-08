#ifndef _INCLUDED_LOCATION_HELPERS_HPP
#define _INCLUDED_LOCATION_HELPERS_HPP

#include "lochash/lochash.hpp"
#include <algorithm>
#include <tuple>
#include <vector>

namespace lochash
{
	namespace detail
	{
		// Helper function to check if a single coordinate is within bounds
		template <typename CoordinateType>
		bool within_bounds(CoordinateType coordinate, CoordinateType lower_bound, CoordinateType upper_bound)
		{
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			return (lower_bound <= coordinate && coordinate <= upper_bound);
		}

		// Helper function to check if coordinates are within bounds using arrays
		template <typename CoordinateType, size_t Dimensions>
		bool within_bounds(const std::array<CoordinateType, Dimensions> & coordinates,
		                   const std::array<CoordinateType, Dimensions> & lower_bounds,
		                   const std::array<CoordinateType, Dimensions> & upper_bounds)
		{
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			for (size_t i = 0; i < Dimensions; ++i) {
				if (!within_bounds(coordinates[i], lower_bounds[i], upper_bounds[i])) {
					return false;
				}
			}
			return true;
		}
	} // namespace detail

	/**
	 * Query objects within a bounding box defined by lower and upper bounds.
	 *
	 * @tparam Precision The precision value for quantization. Must be a power of two.
	 * @tparam CoordinateType The type of the coordinates.
	 * @tparam Dimensions The number of dimensions.
	 * @tparam ObjectType The type of the objects associated with the coordinates.
	 * @param locationHash The LocationHash to query.
	 * @param lower_bounds The lower bounds of the bounding box.
	 * @param upper_bounds The upper bounds of the bounding box.
	 * @return A vector of pointers to objects within the bounding box.
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions, typename ObjectType>
	std::vector<ObjectType *>
	query_bounding_box(const LocationHash<Precision, CoordinateType, Dimensions, ObjectType> & locationHash,
	                   const std::array<CoordinateType, Dimensions> &                          lower_bounds,
	                   const std::array<CoordinateType, Dimensions> &                          upper_bounds)
	{
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");

		std::vector<ObjectType *> result;

		// Generate all hash keys within the specified range
		auto keys = generate_all_quantized_coordinates_within_range<Precision, CoordinateType, Dimensions>(
		    lower_bounds, upper_bounds);

		const auto & locationHashData = locationHash.get_data();
		for (const auto & key : keys) {
			const auto it = locationHashData.find(key);
			if (it != locationHashData.end()) {
				for (const auto & [coordinates, object] : it->second) {
					if (detail::within_bounds(coordinates, lower_bounds, upper_bounds)) {
						result.push_back(object);
					}
				}
			}
		}

		return result;
	}
} // namespace lochash

#endif //_INCLUDED_LOCATION_HELPERS_HPP
