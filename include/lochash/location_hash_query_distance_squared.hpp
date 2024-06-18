#ifndef LOCATION_HASH_QUERY_DISTANCE_HPP
#define LOCATION_HASH_QUERY_DISTANCE_HPP

#include "location_hash.hpp"
#include <cmath>
#include <tuple>
#include <vector>

namespace lochash
{
	namespace detail
	{
		// Helper function to calculate squared difference between coordinates
		template <typename CoordinateType>
		CoordinateType squared_difference(CoordinateType a, CoordinateType b)
		{
			return static_cast<CoordinateType>(std::pow(a - b, 2));
		}

		// Recursive calculation of distance squared
		template <typename CoordinateType, std::size_t Index = 0, typename... Args>
		typename std::enable_if<Index == sizeof...(Args), CoordinateType>::type
		calculate_distance_squared(const std::vector<CoordinateType> &, const std::tuple<Args...> &)
		{
			return 0;
		}

		template <typename CoordinateType, std::size_t Index = 0, typename... Args>
		    typename std::enable_if <
		    Index<sizeof...(Args), CoordinateType>::type
		    calculate_distance_squared(const std::vector<CoordinateType> & point1, const std::tuple<Args...> & point2)
		{
			return squared_difference(point1[Index], std::get<Index>(point2)) +
			       calculate_distance_squared<CoordinateType, Index + 1, Args...>(point1, point2);
		}
	} // namespace detail

	// Function to query objects within a certain distance from a point
	template <std::size_t Precision, typename CoordinateType, typename ObjectType, typename... Args>
	std::vector<ObjectType *>
	query_within_distance(const LocationHash<Precision, CoordinateType, ObjectType> & locationHash,
	                      const std::tuple<CoordinateType, Args...> & center, CoordinateType radius)
	{
		std::vector<ObjectType *> result;
		CoordinateType            radius_squared = radius * radius;

		const auto & locationHashData = locationHash.get_data();
		for (const auto & [key, bucket] : locationHashData) {
			for (const auto & [coordinates, object] : bucket) {
				if (detail::calculate_distance_squared<CoordinateType>(coordinates, center) <= radius_squared) {
					result.push_back(object);
				}
			}
		}

		return result;
	}
} // namespace lochash

#endif // LOCATION_HASH_QUERY_DISTANCE_HPP
