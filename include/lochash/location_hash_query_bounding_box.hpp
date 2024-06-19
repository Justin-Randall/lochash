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
			return (lower_bound <= coordinate && coordinate <= upper_bound);
		}

		// Helper function to check if coordinates are within bounds using tuples
		template <typename CoordinateType, typename... BoundsArgs, std::size_t... Is>
		bool within_bounds(const std::vector<CoordinateType> &               coordinates,
		                   const std::tuple<CoordinateType, BoundsArgs...> & lower_bounds,
		                   const std::tuple<CoordinateType, BoundsArgs...> & upper_bounds, std::index_sequence<Is...>)
		{
			return (... && within_bounds(coordinates[Is], std::get<Is>(lower_bounds), std::get<Is>(upper_bounds)));
		}
	} // namespace detail

	// Function to query objects within a bounding box
	template <std::size_t Precision, typename CoordinateType, typename ObjectType, typename... Args>
	std::vector<ObjectType *>
	query_bounding_box(const LocationHash<Precision, CoordinateType, ObjectType> & locationHash,
	                   const std::tuple<CoordinateType, Args...> &                 lower_bounds,
	                   const std::tuple<CoordinateType, Args...> &                 upper_bounds)
	{
		std::vector<ObjectType *> result;

		const auto & locationHashData = locationHash.get_data();
		for (const auto & [key, bucket] : locationHashData) {
			for (const auto & [coordinates, object] : bucket) {
				if (detail::within_bounds(coordinates, lower_bounds, upper_bounds,
				                          std::index_sequence_for<CoordinateType, Args...>{})) {
					result.push_back(object);
				}
			}
		}

		return result;
	}
} // namespace lochash

#endif //_INCLUDED_LOCATION_HELPERS_HPP
