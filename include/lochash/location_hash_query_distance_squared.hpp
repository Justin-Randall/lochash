#ifndef LOCATION_HASH_QUERY_DISTANCE_HPP
#define LOCATION_HASH_QUERY_DISTANCE_HPP

#include "location_hash.hpp"
#include <cmath>
#include <vector>

namespace lochash
{
	namespace detail
	{
		// Helper function to calculate squared difference between coordinates
		template <typename CoordinateType>
		CoordinateType squared_difference(CoordinateType a, CoordinateType b)
		{
			return (a - b) * (a - b);
		}

		template <typename CoordinateType, std::size_t Dimensions>
		CoordinateType calculate_distance_squared(const std::array<CoordinateType, Dimensions> & point1,
		                                          const std::array<CoordinateType, Dimensions> & point2)
		{
			CoordinateType distance_squared = 0;
			for (std::size_t i = 0; i < Dimensions; ++i) {
				distance_squared += squared_difference(point1[i], point2[i]);
			}
			return distance_squared;
		}
	} // namespace detail

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
	template <std::size_t Precision, typename CoordinateType, std::size_t Dimensions>
	std::vector<std::size_t>
	generate_all_hash_keys_within_distance(const std::array<CoordinateType, Dimensions> & center, CoordinateType radius)
	{
		std::array<CoordinateType, Dimensions> lower_bounds;
		std::array<CoordinateType, Dimensions> upper_bounds;

		for (std::size_t i = 0; i < Dimensions; ++i) {
			lower_bounds[i] = center[i] - radius;
			upper_bounds[i] = center[i] + radius;
		}

		return generate_all_hash_keys_within_range<Precision, CoordinateType, Dimensions>(lower_bounds, upper_bounds);
	}

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
	template <std::size_t Precision, typename CoordinateType, std::size_t Dimensions, typename ObjectType>
	std::vector<ObjectType *>
	query_within_distance(const LocationHash<Precision, CoordinateType, Dimensions, ObjectType> & locationHash,
	                      const std::array<CoordinateType, Dimensions> & center, CoordinateType radius)
	{
		std::vector<ObjectType *> result;
		CoordinateType            radius_squared = radius * radius;

		// Generate all hash keys within the specified distance
		auto hash_keys = generate_all_hash_keys_within_distance<Precision, CoordinateType, Dimensions>(center, radius);

		const auto & locationHashData = locationHash.get_data();
		for (const auto & hash_key : hash_keys) {
			auto it = locationHashData.find(hash_key);
			if (it != locationHashData.end()) {
				for (const auto & [coordinates, object] : it->second) {
					if (detail::calculate_distance_squared<CoordinateType, Dimensions>(coordinates, center) <=
					    radius_squared) {
						result.push_back(object);
					}
				}
			}
		}

		return result;
	}
} // namespace lochash

#endif // LOCATION_HASH_QUERY_DISTANCE_HPP
