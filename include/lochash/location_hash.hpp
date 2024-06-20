#ifndef _INCLUDED_location_hash_HPP
#define _INCLUDED_location_hash_HPP

#include "location_hash_algorithm.hpp"
#include <algorithm>
#include <array>
#include <unordered_map>
#include <vector>

namespace lochash
{
	/**
	 * LocationHash class to manage spatial hashing of n-dimensional coordinates.
	 *
	 * @tparam Precision The precision value for quantization. Must be a power of two.
	 * @tparam CoordinateType The type of the coordinates. Must be an arithmetic type.
	 * @tparam Dimensions The number of dimensions for the coordinates.
	 * @tparam ObjectType The type of the associated object. Defaults to void if no associated object is stored.
	 */
	template <std::size_t Precision, typename CoordinateType, std::size_t Dimensions, typename ObjectType = void>
	class LocationHash
	{
	  public:
		static constexpr size_t dimension_count = Dimensions;
		using CoordinateVector                  = std::vector<CoordinateType>;
		using BucketContent                     = std::vector<std::pair<CoordinateVector, ObjectType *>>;

		/**
		 * Adds coordinates and optionally an associated object pointer to the appropriate bucket.
		 *
		 * @param object Pointer to the associated object (optional, only if ObjectType is not void).
		 * @param coordinates Variadic coordinate inputs.
		 */
		template <typename... Args, typename = std::enable_if_t<!std::is_void<ObjectType>::value>>
		void add(ObjectType * object, const Args &... coordinates)
		{
			static_assert(sizeof...(Args) == dimension_count,
			              "The number of coordinates must match the dimension count.");
			static_assert(are_all_same<CoordinateType, Args...>::value, "All coordinates must be of the same type.");
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			CoordinateVector coord_vec = {static_cast<CoordinateType>(coordinates)...};
			std::size_t      hash_key  = generate_hash<Precision>(coordinates...);
			data_[hash_key].emplace_back(coord_vec, object);
		}

		template <typename... Args, typename = std::enable_if_t<std::is_void<ObjectType>::value>>
		void add(const Args &... coordinates)
		{
			static_assert(sizeof...(Args) == dimension_count,
			              "The number of coordinates must match the dimension count.");
			static_assert(are_all_same<CoordinateType, Args...>::value, "All coordinates must be of the same type.");
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			CoordinateVector coord_vec = {static_cast<CoordinateType>(coordinates)...};
			std::size_t      hash_key  = generate_hash<Precision>(coordinates...);
			data_[hash_key].emplace_back(coord_vec, nullptr);
		}

		/**
		 * Retrieves all coordinates and associated objects within a certain bucket.
		 *
		 * @param coordinates Variadic coordinate inputs to determine the bucket.
		 * @return A reference to the bucket content (vector of coordinates and associated objects).
		 */
		template <typename... Args>
		const BucketContent & query(const Args &... coordinates) const
		{
			static_assert(sizeof...(Args) == dimension_count,
			              "The number of coordinates must match the dimension count.");
			static_assert(are_all_same<CoordinateType, Args...>::value, "All coordinates must be of the same type.");
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			std::size_t hash_key = generate_hash<Precision>(coordinates...);
			auto        it       = data_.find(hash_key);
			if (it != data_.end()) {
				return it->second;
			} else {
				static const BucketContent empty_bucket;
				return empty_bucket;
			}
		}

		/**
		 * Removes a coordinate and optionally an associated object from the appropriate bucket.
		 *
		 * @param coordinates Variadic coordinate inputs.
		 * @return True if an item was removed, false otherwise.
		 */
		template <typename... Args, typename = std::enable_if_t<std::is_void<ObjectType>::value>>
		bool remove(const Args &... coordinates)
		{
			static_assert(sizeof...(Args) == dimension_count,
			              "The number of coordinates must match the dimension count.");
			static_assert(are_all_same<CoordinateType, Args...>::value, "All coordinates must be of the same type.");
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			std::size_t hash_key = generate_hash<Precision>(coordinates...);
			auto        it       = data_.find(hash_key);
			if (it != data_.end()) {
				auto & bucket = it->second;
				for (auto bucket_it = bucket.begin(); bucket_it != bucket.end(); ++bucket_it) {
					// Remove by coordinate comparison with epsilon for floating points
					bool                                        match       = true;
					std::array<CoordinateType, sizeof...(Args)> coord_array = {
					    static_cast<CoordinateType>(coordinates)...};
					for (std::size_t i = 0; i < coord_array.size(); ++i) {
						if constexpr (std::is_floating_point<CoordinateType>::value) {
							if (std::fabs(bucket_it->first[i] - coord_array[i]) >
							    std::numeric_limits<CoordinateType>::epsilon()) {
								match = false;
								break;
							}
						} else {
							if (bucket_it->first[i] != coord_array[i]) {
								match = false;
								break;
							}
						}
					}
					if (match) {
						bucket.erase(bucket_it);
						if (bucket.empty()) {
							data_.erase(it);
						}
						return true;
					}
				}
			}
			return false;
		}

		template <typename... Args, typename = std::enable_if_t<!std::is_void<ObjectType>::value>>
		bool remove(ObjectType * object, const Args &... coordinates)
		{
			static_assert(sizeof...(Args) == dimension_count,
			              "The number of coordinates must match the dimension count.");
			static_assert(are_all_same<CoordinateType, Args...>::value, "All coordinates must be of the same type.");
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			std::size_t hash_key = generate_hash<Precision>(coordinates...);
			auto        it       = data_.find(hash_key);
			if (it != data_.end()) {
				auto & bucket = it->second;
				for (auto bucket_it = bucket.begin(); bucket_it != bucket.end(); ++bucket_it) {
					// Remove by object pointer comparison
					if (bucket_it->second == object) {
						bucket.erase(bucket_it);
						if (bucket.empty()) {
							data_.erase(it);
						}
						return true;
					}
				}
			}
			return false;
		}

		/**
		 * Moves a coordinate and optionally an associated object from one bucket to another.
		 *
		 * @param old_coordinates Variadic coordinate inputs for the current location.
		 * @param new_coordinates Variadic coordinate inputs for the new location.
		 * @return True if an item was moved, false otherwise.
		 */
		template <typename... OldArgs, typename... NewArgs>
		bool move(const std::tuple<OldArgs...> & old_coordinates, const std::tuple<NewArgs...> & new_coordinates)
		{
			static_assert(sizeof...(OldArgs) == dimension_count,
			              "The number of old coordinates must match the dimension count.");
			static_assert(sizeof...(NewArgs) == dimension_count,
			              "The number of new coordinates must match the dimension count.");
			static_assert(are_all_same<CoordinateType, OldArgs...>::value,
			              "All old coordinates must be of the same type.");
			static_assert(are_all_same<CoordinateType, NewArgs...>::value,
			              "All new coordinates must be of the same type.");
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			return move_impl(std::index_sequence_for<OldArgs...>(), std::index_sequence_for<NewArgs...>(),
			                 old_coordinates, new_coordinates);
		}

		template <typename... OldArgs, typename... NewArgs,
		          typename = std::enable_if_t<!std::is_void<ObjectType>::value>>
		bool move(ObjectType * object, const std::tuple<OldArgs...> & old_coordinates,
		          const std::tuple<NewArgs...> & new_coordinates)
		{
			static_assert(sizeof...(OldArgs) == dimension_count,
			              "The number of old coordinates must match the dimension count.");
			static_assert(sizeof...(NewArgs) == dimension_count,
			              "The number of new coordinates must match the dimension count.");
			static_assert(are_all_same<CoordinateType, OldArgs...>::value,
			              "All old coordinates must be of the same type.");
			static_assert(are_all_same<CoordinateType, NewArgs...>::value,
			              "All new coordinates must be of the same type.");
			static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

			return move_impl(std::index_sequence_for<OldArgs...>(), std::index_sequence_for<NewArgs...>(), object,
			                 old_coordinates, new_coordinates);
		}

		/**
		 * Returns the underlying data map.
		 *
		 * @return The underlying data map.
		 */
		const std::unordered_map<std::size_t, BucketContent> & get_data() const { return data_; }

		void clear() { data_.clear(); }

	  private:
		template <std::size_t... OldIndices, std::size_t... NewIndices, typename... OldArgs, typename... NewArgs>
		bool move_impl(std::index_sequence<OldIndices...>, std::index_sequence<NewIndices...>,
		               const std::tuple<OldArgs...> & old_coordinates, const std::tuple<NewArgs...> & new_coordinates)
		{
			if (remove(std::get<OldIndices>(old_coordinates)...)) {
				add(std::get<NewIndices>(new_coordinates)...);
				return true;
			}
			return false;
		}

		template <std::size_t... OldIndices, std::size_t... NewIndices, typename... OldArgs, typename... NewArgs>
		bool move_impl(std::index_sequence<OldIndices...>, std::index_sequence<NewIndices...>, ObjectType * object,
		               const std::tuple<OldArgs...> & old_coordinates, const std::tuple<NewArgs...> & new_coordinates)
		{
			if (remove(object, std::get<OldIndices>(old_coordinates)...)) {
				add(object, std::get<NewIndices>(new_coordinates)...);
				return true;
			}
			return false;
		}

		std::unordered_map<std::size_t, BucketContent> data_;
	};
} // namespace lochash

#endif //_INCLUDED_location_hash_HPP
