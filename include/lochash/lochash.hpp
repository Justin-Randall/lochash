#ifndef _INCLUDED_lochash_hpp
#define _INCLUDED_lochash_hpp

#include <cmath>
#include <cstdint>
#include <functional>
#include <type_traits>

/**
 * Combines the hash of a single value with a seed value.
 *
 * @tparam T The type of the value to hash. Must be hashable by std::hash.
 * @param seed The current seed value.
 * @param value The value to hash.
 * @return A combined hash value.
 */
template <typename T> std::size_t hash_combine(std::size_t seed, const T & value)
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
	//   more evenly distributed hash result
	// - Div, for example, can burn 39 cycles on Skylake, while shifts and XORs can be done in 1 cycle.
	return seed ^ (hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

/**
 * Recursively combines the hash of multiple values with a seed value.
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
template <typename T, std::size_t Precision> constexpr std::size_t quantize_value(T value)
{
	static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");

	// Using bitwise AND with ~(Precision - 1) to quantize the value
	// This is more performant than using division or modulo operations because
	// bitwise operations are generally faster and have lower latency in modern CPUs.
	// Division and modulo operations typically take multiple CPU cycles to complete,
	// whereas bitwise operations usually execute in a single cycle.
	return static_cast<std::size_t>(value) & ~(Precision - 1);
}

// Helper metafunction to check if all types are the same
template <typename T, typename... Args> struct are_all_same;

template <typename T> struct are_all_same<T> : std::true_type {
};

template <typename T, typename U, typename... Args> struct are_all_same<T, U, Args...> : std::false_type {
};

template <typename T, typename... Args> struct are_all_same<T, T, Args...> : are_all_same<T, Args...> {
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

	// Initial seed with quantized first value
	// Quantization ensures values are grouped into buckets defined by Precision
	std::size_t seed = hash_combine(0, quantize_value<T, Precision>(value));

	// Recursively combine the hash of remaining quantized values
	// The fold expression ((seed = hash_combine(seed, quantize_value<T, Precision>(args))), ...)
	// ensures each argument is processed in order, combining their hashes into the final seed.
	((seed = hash_combine(seed, quantize_value<T, Precision>(args))), ...);
	return seed;
}

#endif //_INCLUDED_lochash_hpp
