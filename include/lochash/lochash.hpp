#ifndef _INCLUDED_lochash_hpp
#define _INCLUDED_lochash_hpp

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <unordered_map>
#include <vector>

/**
 * Combines the hash of a single value with a seed value.
 *
 * @tparam T The type of the value to hash. Must be hashable by std::hash.
 * @param seed The current seed value.
 * @param value The value to hash.
 * @return A combined hash value.
 */
template <typename T>
std::size_t hash_combine(std::size_t seed, const T & value)
{
	std::hash<T> hasher;
	// Combines the current seed with the hash of the value using bitwise operations
	//
	// Explanation:
	// 1. hasher(value): Computes the hash of the input value.
	// 2. seed << 6: Left shift the seed by 6 bits. This is equivalent to multiplying the seed by 64.
	// 3. seed >> 2: Right shift the seed by 2 bits. This is equivalent to dividing the seed by 4.
	// 4. 0x9e3779b9: A large constant (part of the golden ratio) used to distribute the hash values more uniformly.
	// 5. The final result is obtained by combining these values using bitwise XOR (^) and addition (+) operations.
	//
	// Why bitwise operations:
	// - Bitwise operations (<<, >>, ^) are generally faster than arithmetic operations (e.g., multiplication, division)
	//   because they operate directly on the binary representation of the numbers.
	// - They help in spreading out the bits more uniformly, which reduces the chances of hash collisions.
	// - The combination of shifts and XOR operations helps in mixing the bits of the hash value and seed, leading to a
	// more
	//   evenly distributed hash result.
	return seed ^ (hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

/**
 * Recursively combines the hash of multiple values with a seed value.
 * This is a compile-time recursion using variadic templates.
 *
 * @tparam T The type of the first value to hash.
 * @tparam Args The types of the remaining values to hash.
 * @param seed The current seed value.
 * @param value The first value to hash.
 * @param args The remaining values to hash.
 * @return A combined hash value.
 */
template <typename T, typename... Args>
std::size_t hash_combine(std::size_t seed, const T & value, const Args &... args)
{
	seed = hash_combine(seed, value);
	// Recursively combine hashes of remaining values
	return hash_combine(seed, args...);
}

/**
 * Quantizes a value based on the specified precision.
 * This function uses bitwise operations for performance, avoiding division.
 *
 * @tparam T The type of the value to quantize. Must be an arithmetic type.
 * @tparam Precision The precision value. Must be a power of two.
 * @param value The value to quantize.
 * @return The quantized value.
 */
template <typename T, std::size_t Precision>
constexpr std::size_t quantize_value(T value)
{
	static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");
	// Using bitwise AND with ~(Precision - 1) to quantize the value
	// This is more performant than using division or modulo operations because
	// bitwise operations are generally faster and have lower latency in modern CPUs.
	// Division and modulo operations typically take multiple CPU cycles to complete,
	// whereas bitwise operations usually execute in a single cycle.
	return static_cast<std::size_t>(value) & ~(Precision - 1);
}

// Helper metafunction to check if all types in a parameter pack are the same
template <typename T, typename... Args>
struct are_all_same;

template <typename T>
struct are_all_same<T> : std::true_type {
};

template <typename T, typename U, typename... Args>
struct are_all_same<T, U, Args...> : std::false_type {
};

template <typename T, typename... Args>
struct are_all_same<T, T, Args...> : are_all_same<T, Args...> {
};

/**
 * Generates a hash from an arbitrary number of parameters with precision.
 * All input parameters must be of the same type, and this is enforced at compile time.
 *
 * @tparam Precision The precision value for quantization. Must be a power of two.
 * @tparam T The type of the first value to hash. Must be an arithmetic type.
 * @tparam Args The types of the remaining values to hash. Must be the same as T.
 * @param value The first value to hash.
 * @param args The remaining values to hash.
 * @return A combined hash value.
 */
template <std::size_t Precision, typename T, typename... Args>
std::size_t generate_hash(const T & value, const Args &... args)
{
	static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");
	static_assert(are_all_same<T, Args...>::value, "All arguments must be of the same type");
	static_assert((Precision & (Precision - 1)) == 0, "Precision must be a power of two");

	// Initial seed with quantized first value
	// Quantization ensures values are grouped into buckets defined by Precision
	std::size_t seed = hash_combine(0, quantize_value<T, Precision>(value));
	// Recursively combine the hash of remaining quantized values
	// The fold expression ((seed = hash_combine(seed, quantize_value<T, Precision>(args))), ...)
	// ensures each argument is processed in order, combining their hashes into the final seed.
	((seed = hash_combine(seed, quantize_value<T, Precision>(args))), ...);
	return seed;
}

/**
 * LocationHash class to manage spatial hashing of n-dimensional coordinates.
 *
 * @tparam Precision The precision value for quantization. Must be a power of two.
 * @tparam CoordinateType The type of the coordinates. Must be an arithmetic type.
 * @tparam ObjectType The type of the associated object. Defaults to void if no associated object is stored.
 */
template <std::size_t Precision, typename CoordinateType, typename ObjectType = void>
class LocationHash
{
  public:
	using CoordinateVector = std::vector<CoordinateType>;
	using BucketContent    = std::vector<std::pair<CoordinateVector, ObjectType *>>;

	/**
	 * Adds coordinates and optionally an associated object pointer to the appropriate bucket.
	 *
	 * @param object Pointer to the associated object (optional, only if ObjectType is not void).
	 * @param coordinates Variadic coordinate inputs.
	 */
	template <typename... Args, typename = std::enable_if_t<!std::is_void<ObjectType>::value>>
	void add(ObjectType * object, const Args &... coordinates)
	{
		static_assert(sizeof...(Args) > 0, "At least one coordinate must be provided.");
		static_assert(are_all_same<CoordinateType, Args...>::value, "All coordinates must be of the same type.");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

		CoordinateVector coord_vec = {static_cast<CoordinateType>(coordinates)...};
		std::size_t      hash_key  = generate_hash<Precision>(coordinates...);
		data_[hash_key].emplace_back(coord_vec, object);
	}

	template <typename... Args, typename = std::enable_if_t<std::is_void<ObjectType>::value>>
	void add(const Args &... coordinates)
	{
		static_assert(sizeof...(Args) > 0, "At least one coordinate must be provided.");
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
		static_assert(sizeof...(Args) > 0, "At least one coordinate must be provided.");
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
		static_assert(sizeof...(Args) > 0, "At least one coordinate must be provided.");
		static_assert(are_all_same<CoordinateType, Args...>::value, "All coordinates must be of the same type.");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

		std::size_t hash_key = generate_hash<Precision>(coordinates...);
		auto        it       = data_.find(hash_key);
		if (it != data_.end()) {
			auto & bucket = it->second;
			for (auto bucket_it = bucket.begin(); bucket_it != bucket.end(); ++bucket_it) {
				// Remove by coordinate comparison with epsilon for floating points
				bool                                        match       = true;
				std::array<CoordinateType, sizeof...(Args)> coord_array = {static_cast<CoordinateType>(coordinates)...};
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
		static_assert(sizeof...(Args) > 0, "At least one coordinate must be provided.");
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
		static_assert(sizeof...(OldArgs) > 0, "At least one old coordinate must be provided.");
		static_assert(sizeof...(NewArgs) > 0, "At least one new coordinate must be provided.");
		static_assert(are_all_same<CoordinateType, OldArgs...>::value, "All old coordinates must be of the same type.");
		static_assert(are_all_same<CoordinateType, NewArgs...>::value, "All new coordinates must be of the same type.");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

		return move_impl(std::index_sequence_for<OldArgs...>(), std::index_sequence_for<NewArgs...>(), old_coordinates,
		                 new_coordinates);
	}

	template <typename... OldArgs, typename... NewArgs, typename = std::enable_if_t<!std::is_void<ObjectType>::value>>
	bool move(ObjectType * object, const std::tuple<OldArgs...> & old_coordinates,
	          const std::tuple<NewArgs...> & new_coordinates)
	{
		static_assert(sizeof...(OldArgs) > 0, "At least one old coordinate must be provided.");
		static_assert(sizeof...(NewArgs) > 0, "At least one new coordinate must be provided.");
		static_assert(are_all_same<CoordinateType, OldArgs...>::value, "All old coordinates must be of the same type.");
		static_assert(are_all_same<CoordinateType, NewArgs...>::value, "All new coordinates must be of the same type.");
		static_assert(std::is_arithmetic<CoordinateType>::value, "CoordinateType must be an arithmetic type.");

		return move_impl(std::index_sequence_for<OldArgs...>(), std::index_sequence_for<NewArgs...>(), object,
		                 old_coordinates, new_coordinates);
	}

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

#endif //_INCLUDED_lochash_hpp
