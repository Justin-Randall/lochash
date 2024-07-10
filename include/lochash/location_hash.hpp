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
	 *
	 * @see https://github.com/Justin-Randall/lochash/blob/main/README.md
	 */
	template <size_t Precision, typename CoordinateType, size_t Dimensions, typename ObjectType = void>
	class LocationHash
	{
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

	  public:
		static constexpr size_t dimension_count = Dimensions;
		using CoordinateArray                   = std::array<CoordinateType, Dimensions>;
		using BucketContent                     = std::vector<std::pair<CoordinateArray, ObjectType *>>;

		/**
		 * Adds coordinates and optionally an associated object pointer to the appropriate bucket.
		 *
		 * @param object Pointer to the associated object (optional, only if ObjectType is not void).
		 * @param coordinates Array of coordinate inputs.
		 */
		void add(ObjectType * object, const CoordinateArray & coordinates)
		{
			size_t hash_key = generate_hash<Precision>(coordinates);
			data_[hash_key].emplace_back(coordinates, object);
		}

		/**
		 * Adds coordinates to the appropriate bucket.
		 *
		 * @param coordinates Array of coordinate inputs.
		 */
		void add(const CoordinateArray & coordinates)
		{
			size_t hash_key = generate_hash<Precision>(coordinates);
			data_[hash_key].emplace_back(coordinates, nullptr);
		}

		/**
		 * Retrieves all coordinates and associated objects within a certain bucket.
		 *
		 * @param coordinates Array of coordinate inputs to determine the bucket.
		 * @return A reference to the bucket content (vector of coordinates and associated objects).
		 */
		const BucketContent & query(const CoordinateArray & coordinates) const
		{
			size_t     hash_key = generate_hash<Precision>(coordinates);
			const auto it       = data_.find(hash_key);
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
		 * @param coordinates Array of coordinate inputs.
		 * @return True if an item was removed, false otherwise.
		 */
		bool remove(const CoordinateArray & coordinates)
		{
			size_t     hash_key = generate_hash<Precision>(coordinates);
			const auto it       = data_.find(hash_key);
			if (it != data_.end()) {
				auto & bucket = it->second;
				for (auto bucket_it = bucket.begin(); bucket_it != bucket.end(); ++bucket_it) {
					if (coordinates_match(bucket_it->first, coordinates)) {
						// safe to erase in the loop, going to return immediately
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
		 * Removes a coordinate and optionally an associated object from the appropriate bucket.
		 *
		 * @param object Pointer to the associated object.
		 * @param coordinates Array of coordinate inputs.
		 * @return True if an item was removed, false otherwise.
		 */
		bool remove(ObjectType * object, const CoordinateArray & coordinates)
		{
			size_t     hash_key = generate_hash<Precision>(coordinates);
			const auto it       = data_.find(hash_key);
			if (it != data_.end()) {
				auto & bucket = it->second;
				for (auto bucket_it = bucket.begin(); bucket_it != bucket.end(); ++bucket_it) {
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
		 * @param old_coordinates Array of coordinate inputs for the current location.
		 * @param new_coordinates Array of coordinate inputs for the new location.
		 * @return True if an item was moved, false otherwise.
		 */
		bool move(const CoordinateArray & old_coordinates, const CoordinateArray & new_coordinates)
		{
			if (buckets_match(old_coordinates, new_coordinates)) {
				return false;
			}

			if (remove(old_coordinates)) {
				add(new_coordinates);
				return true;
			}
			return false;
		}

		/**
		 * @brief Moves a coordinate and optionally an associated object from one bucket to another.
		 *
		 * @param object
		 * @param old_coordinates
		 * @param new_coordinates
		 * @return true
		 * @return false
		 */
		bool move(ObjectType * object, const CoordinateArray & old_coordinates, const CoordinateArray & new_coordinates)
		{
			if (buckets_match(old_coordinates, new_coordinates)) {
				return false;
			}

			if (remove(object, old_coordinates)) {
				add(object, new_coordinates);
				return true;
			}
			return false;
		}

		/**
		 * Returns the underlying data map.
		 *
		 * @return The underlying data map.
		 */
		const std::unordered_map<size_t, BucketContent> & get_data() const { return data_; }

		/**
		 * Clears all data from the LocationHash.
		 */
		void clear() { data_.clear(); }

	  private:
		bool buckets_match(const CoordinateArray & coords1, const CoordinateArray & coords2) const
		{
			const auto old_coordinates_hash = generate_hash<Precision>(coords1);
			const auto new_coordinates_hash = generate_hash<Precision>(coords2);
			if (old_coordinates_hash == new_coordinates_hash) {
				return true;
			}
			return false;
		}
		bool coordinates_match(const CoordinateArray & coords1, const CoordinateArray & coords2) const
		{
			for (size_t i = 0; i < Dimensions; ++i) {
				if constexpr (std::is_floating_point<CoordinateType>::value) {
					if (std::fabs(coords1[i] - coords2[i]) > std::numeric_limits<CoordinateType>::epsilon()) {
						return false;
					}
				} else {
					if (coords1[i] != coords2[i]) {
						return false;
					}
				}
			}
			return true;
		}

		std::unordered_map<size_t, BucketContent> data_;
	};
} // namespace lochash

#endif //_INCLUDED_location_hash_HPP
