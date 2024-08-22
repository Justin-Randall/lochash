#ifndef _INCLUDED_location_hash_HPP
#define _INCLUDED_location_hash_HPP

#include "location_hash_algorithm.hpp"
#include "location_hash_quantized_coordinate.hpp"
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
	template <
	    size_t Precision, typename CoordinateType, size_t Dimensions, typename ObjectType = void,
	    typename QuantizedCoordinateIntegerType = int64_t,
	    typename Hash =
	        std::hash<QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>>,
	    typename KeyEqual =
	        std::equal_to<QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>>,
	    typename Allocator = std::allocator<
	        std::pair<const QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>,
	                  std::vector<std::pair<std::array<CoordinateType, Dimensions>, ObjectType *>>>>>
	class LocationHash
	{
		static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

	  public:
		static constexpr size_t dimension_count = Dimensions;
		using CoordinateArray                   = std::array<CoordinateType, Dimensions>;
		using BucketContent                     = std::vector<std::pair<CoordinateArray, ObjectType *>>;
		using CoordinateMap                     = std::unordered_map<
            QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>, BucketContent,
            Hash, KeyEqual, Allocator>;
		using QuantizedCoordinateType =
		    QuantizedCoordinate<Precision, CoordinateType, Dimensions, QuantizedCoordinateIntegerType>;

		/**
		 * Adds coordinates and optionally an associated object pointer to the appropriate bucket.
		 *
		 * @param object Pointer to the associated object (optional, only if ObjectType is not void).
		 * @param coordinates Array of coordinate inputs.
		 */
		void add(ObjectType * object, const CoordinateArray & coordinates)
		{
			data_[QuantizedCoordinate<Precision, CoordinateType, Dimensions>(coordinates)].emplace_back(coordinates,
			                                                                                            object);
		}

		/**
		 * Adds coordinates and optionally an associated object pointer to the appropriate buckets.
		 * Used when an object has a size and needs to be added to multiple buckets. Even if an
		 * object is not as large as a bucket, it may be near an edge or corner and overlap multiple
		 * buckets. This optimizes queries so searching one bucket will return the object, even if its
		 * center is in another bucket.
		 *
		 * @param object Pointer to the associated object (optional, only if ObjectType is not void).
		 * @param coordinates Array of coordinate inputs.
		 * @param radius The radius of the bucket.
		 */
		std::vector<QuantizedCoordinateType> add(ObjectType * object, const CoordinateArray & coordinates,
		                                         CoordinateType radius)
		{
			// generate keys for all the buckets within the radius
			auto keys =
			    generate_all_quantized_coordinates_within_distance<Precision, CoordinateType, Dimensions,
			                                                       QuantizedCoordinateIntegerType>(coordinates, radius);

			for (auto key : keys) {
				data_[key].emplace_back(coordinates, object);
			}

			return keys;
		}

		/**
		 * Adds coordinates to the appropriate bucket.
		 *
		 * @param coordinates Array of coordinate inputs.
		 */
		void add(const CoordinateArray & coordinates)
		{
			// Implicit conversion to QuantizedCoordinate, so fine to use CoordinateArray as a key
			data_[coordinates].emplace_back(coordinates, nullptr);
		}

		/**
		 * Retrieves all coordinates and associated objects within a certain bucket.
		 *
		 * @param coordinates Array of coordinate inputs to determine the bucket.
		 * @return A reference to the bucket content (vector of coordinates and associated objects).
		 */
		const BucketContent & query(const CoordinateArray & coordinates) const
		{
			const auto it = data_.find(QuantizedCoordinate<Precision, CoordinateType, Dimensions>(coordinates));
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
			QuantizedCoordinate key = QuantizedCoordinate<Precision, CoordinateType, Dimensions>(coordinates);
			const auto          it  = data_.find(key);
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
			QuantizedCoordinate key = QuantizedCoordinate<Precision, CoordinateType, Dimensions>(coordinates);
			const auto          it  = data_.find(key);
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
		 * Removes a coordinate and optionally an associated object from the appropriate buckets.
		 *
		 * @param object Pointer to the associated object.
		 * @param coordinates Array of coordinate inputs.
		 * @param radius The radius of the bucket.
		 * @return True if an item was removed, false otherwise.
		 */
		bool remove(ObjectType * object, const CoordinateArray & coordinates, const CoordinateType radius)
		{
			auto keys =
			    generate_all_quantized_coordinates_within_distance<Precision, CoordinateType, Dimensions,
			                                                       QuantizedCoordinateIntegerType>(coordinates, radius);
			bool removed = false;
			for (auto key : keys) {
				const auto it = data_.find(key);
				if (it != data_.end()) {
					auto & bucket = it->second;
					for (auto bucket_it = bucket.begin(); bucket_it != bucket.end(); ++bucket_it) {
						if (bucket_it->second == object) {
							bucket.erase(bucket_it);
							removed = true;
							break;
						}
					}
					if (bucket.empty()) {
						data_.erase(it);
					}
				}
			}
			return removed;
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
		 * @param object Pointer to the associated object.
		 * @param old_coordinates Array of coordinate inputs for the current location.
		 * @param new_coordinates Array of coordinate inputs for the new location.
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
		 * @brief Moves an object. Updates buckets it may be in. Adds new buckets if the radius
		 * 	  of the object's edge extends into new buckets and removes itself from old buckts if
		 * 	  the object's edge no longer extends into them.
		 *
		 * @param object Pointer to the associated object.
		 * @param radius The radius of the object.
		 * @param old_coordinates The old coordinates of the object.
		 * @param new_coordinates The new coordinates of the object.
		 * @return std::vector<QuantizedCoordinateType>
		 */
		std::vector<QuantizedCoordinateType> move(ObjectType * object, const CoordinateType & radius,
		                                          const CoordinateArray & old_coordinates,
		                                          const CoordinateArray & new_coordinates)
		{
			// from the top down: find early-out opportunities that spend less time

			// if the coordinates are the same, we don't need to do anything
			if (coordinates_match(old_coordinates, new_coordinates)) {
				return generate_all_quantized_coordinates_within_distance<Precision, CoordinateType, Dimensions,
				                                                          QuantizedCoordinateIntegerType>(
				    old_coordinates, radius);
			}

			// see if the keys and buckets are the same
			QuantizedCoordinate q_old = QuantizedCoordinate<Precision, CoordinateType, Dimensions>(old_coordinates);
			QuantizedCoordinate q_new = QuantizedCoordinate<Precision, CoordinateType, Dimensions>(new_coordinates);
			if (q_old == q_new) {
				// same buckets, but the new coordinates may include new buckets and exclude others.
				// First make a quick calculation to see if the new buckets are the same as the old buckets.
				// If they match, there is no need to mutate the data structure.
				const auto newBuckets =
				    generate_all_quantized_coordinates_within_distance<Precision, CoordinateType, Dimensions,
				                                                       QuantizedCoordinateIntegerType>(new_coordinates,
				                                                                                       radius);
				const auto oldBuckets =
				    generate_all_quantized_coordinates_within_distance<Precision, CoordinateType, Dimensions,
				                                                       QuantizedCoordinateIntegerType>(old_coordinates,
				                                                                                       radius);
				if (newBuckets == oldBuckets) {
					return newBuckets;
				}
			}

			// less expensive early-outs failed, so we need to actually move the object
			remove(object, old_coordinates, radius);
			auto keys = add(object, new_coordinates, radius);
			return keys;
		}

		/**
		 * Returns the underlying data map.
		 *
		 * @return The underlying data map.
		 */
		// const std::unordered_map<size_t, BucketContent> & get_data() const { return data_; }
		const CoordinateMap & get_data() const { return data_; }

		/**
		 * Clears all data from the LocationHash.
		 */
		void clear() { data_.clear(); }

	  private:
		bool buckets_match(const CoordinateArray & coords1, const CoordinateArray & coords2) const
		{
			// Keep in mind that these are quantized coordinates. The coordinate arrays can be different
			// but the quantized coordinates can be the same and point to the same bucket.
			const auto old_coordinates = QuantizedCoordinate<Precision, CoordinateType, Dimensions>(coords1);
			const auto new_coordinates = QuantizedCoordinate<Precision, CoordinateType, Dimensions>(coords2);
			if (old_coordinates == new_coordinates) {
				return true;
			}
			return false;
		}

		bool coordinates_match(const CoordinateArray & coords1, const CoordinateArray & coords2) const
		{
			// This has not turned up in profiling as a bottleneck, but it could be optimized
			// using SIMD instructions or other techniques.
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

		CoordinateMap data_;
	};
} // namespace lochash

#endif //_INCLUDED_location_hash_HPP
