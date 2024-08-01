#ifndef LOCATION_HASH_QUERY_DISTANCE_HPP
#define LOCATION_HASH_QUERY_DISTANCE_HPP

#include "location_hash.hpp"
#include <cmath>
#include <vector>

namespace lochash
{

	/***
	 * Query objects within a certain distance from a point.
	 *
	 * @tparam Precision The precision value for quantization. Must be a power of two.
	 * @tparam CoordinateType The type of the coordinates.
	 * @tparam Dimensions The number of dimensions.
	 * @tparam ObjectType The type of the objects associated with the coordinates.
	 * @param locationHash The LocationHash to query.
	 * @param center The center point to calculate distance from.
	 * @param radius The distance from the center point.
	 * @return A vector of pointers to objects within the specified distance.
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions, typename ObjectType>
	std::vector<ObjectType *>
	query_within_distance(const LocationHash<Precision, CoordinateType, Dimensions, ObjectType> & locationHash,
	                      const std::array<CoordinateType, Dimensions> & center, CoordinateType radius)
	{
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

		std::vector<ObjectType *> result;
		CoordinateType            radius_squared = radius * radius;

		// Generate all hash keys within the specified distance
		const auto hash_keys =
		    generate_all_hash_keys_within_distance<Precision, CoordinateType, Dimensions>(center, radius);

		const auto & locationHashData = locationHash.get_data();
		for (const auto & hash_key : hash_keys) {
			const auto it = locationHashData.find(hash_key);
			if (it != locationHashData.end()) {
				for (const auto & [coordinates, object] : it->second) {
					if (calculate_distance_squared<CoordinateType, Dimensions>(coordinates, center) <= radius_squared) {
						result.push_back(object);
					}
				}
			}
		}

		return result;
	}
} // namespace lochash

#endif // LOCATION_HASH_QUERY_DISTANCE_HPP
