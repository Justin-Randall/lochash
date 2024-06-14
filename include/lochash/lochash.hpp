#ifndef _INCLUDED_lochash_hpp
#define _INCLUDED_lochash_hpp

#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <type_traits>

// Base case for the variadic template
template <typename T> std::size_t hash_combine(std::size_t seed, const T & value)
{
	std::hash<T> hasher;
	return seed ^ (hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

// Variadic template function to hash multiple values
template <typename T, typename... Args>
std::size_t hash_combine(std::size_t seed, const T & value, const Args &... args)
{
	seed = hash_combine(seed, value);
	return hash_combine(seed, args...);
}

// Function to quantize a value based on the specified precision
template <typename T, std::size_t Precision> constexpr std::size_t quantize_value(T value)
{
	static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");
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

// Variadic template function to generate hash from arbitrary number of
// parameters with precision
template <std::size_t Precision, typename T, typename... Args>
std::size_t generate_hash(const T & value, const Args &... args)
{
	static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");
	static_assert(are_all_same<T, Args...>::value, "All arguments must be of the same type");

	std::size_t seed = hash_combine(0, quantize_value<T, Precision>(value));
	((seed = hash_combine(seed, quantize_value<T, Precision>(args))), ...);
	return seed;
}

#endif //_INCLUDED_lochash_hpp
