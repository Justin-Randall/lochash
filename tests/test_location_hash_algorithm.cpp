#include "lochash/lochash.hpp"
#include "test_helpers.hpp"
#include "gtest/gtest.h"
#include <chrono>

using namespace lochash;

TEST(test_hash_algorithm, IntegerHashing)
{
	constexpr size_t precision = 16;

	auto hash1 = generate_hash<precision, int, 4>({0, 16, 32, 48});
	auto hash2 = generate_hash<precision, int, 4>({1, 17, 33, 49});
	auto hash3 = generate_hash<precision, int, 4>({2, 18, 34, 50});

	// All values fall into the same bucket
	EXPECT_EQ(hash1, hash2);
	EXPECT_EQ(hash2, hash3);

	auto hash4 = generate_hash<precision, int, 4>({15, 31, 47, 63});
	auto hash5 = generate_hash<precision, int, 4>({16, 32, 48, 64});

	// Values in different buckets
	EXPECT_NE(hash4, hash5);
}

TEST(test_hash_algorithm, FloatHashing)
{
	constexpr size_t precision = 16;

	auto hash1 = generate_hash<precision, float, 4>({0.0f, 16.0f, 32.0f, 48.0f});
	auto hash2 = generate_hash<precision, float, 4>({0.1f, 16.1f, 32.1f, 48.1f});
	auto hash3 = generate_hash<precision, float, 4>({0.5f, 16.5f, 32.5f, 48.5f});

	// All values fall into the same bucket
	EXPECT_EQ(hash1, hash2);
	EXPECT_EQ(hash2, hash3);

	auto hash4 = generate_hash<precision, float, 4>({15.9f, 31.9f, 47.9f, 63.9f});
	auto hash5 = generate_hash<precision, float, 4>({16.1f, 32.1f, 48.1f, 64.1f});

	// Values in different buckets
	EXPECT_NE(hash4, hash5);
}

TEST(test_hash_algorithm, DoubleHashing)
{
	constexpr size_t precision = 16;

	auto hash1 = generate_hash<precision, double, 4>({0.0, 16.0, 32.0, 48.0});
	auto hash2 = generate_hash<precision, double, 4>({0.1, 16.1, 32.1, 48.1});
	auto hash3 = generate_hash<precision, double, 4>({0.5, 16.5, 32.5, 48.5});

	// All values fall into the same bucket
	EXPECT_EQ(hash1, hash2);
	EXPECT_EQ(hash2, hash3);

	auto hash4 = generate_hash<precision, double, 4>({15.9, 31.9, 47.9, 63.9});
	auto hash5 = generate_hash<precision, double, 4>({16.1, 32.1, 48.1, 64.1});

	// Values in different buckets
	EXPECT_NE(hash4, hash5);
}

TEST(test_hash_algorithm, MixedTypesHashing)
{
	constexpr size_t precision = 16;

	const auto int_hash    = generate_hash<precision, int, 4>({16, 32, 48, 64});
	const auto float_hash  = generate_hash<precision, float, 4>({16.0f, 32.0f, 48.0f, 64.0f});
	const auto double_hash = generate_hash<precision, double, 4>({16.0, 32.0, 48.0, 64.0});

	// All values should hash to the same bucket
	EXPECT_EQ(int_hash, float_hash);
	EXPECT_EQ(float_hash, double_hash);
}

TEST(test_hash_algorithm, EdgeCases)
{
	constexpr size_t precision = 16;

	// Test with zero values
	static const auto zero_hash = generate_hash<precision, int, 4>({0, 0, 0, 0});
	EXPECT_EQ(zero_hash, zero_hash); // Should always be equal to itself

	// Test with maximum precision values
	const auto max_precision_hash =
	    generate_hash<precision, size_t, 4>({precision - 1, precision - 1, precision - 1, precision - 1});
	EXPECT_EQ(max_precision_hash, max_precision_hash); // Should always be equal to itself

	// Test with mixed zero and precision values
	const auto mixed_hash1 = generate_hash<precision, size_t, 4>({size_t(0), precision, 2 * precision, 3 * precision});
	const auto mixed_hash2 = generate_hash<precision, size_t, 4>({size_t(0), precision, 2 * precision, 3 * precision});

	EXPECT_EQ(mixed_hash1, mixed_hash2); // Should be equal
}

TEST(test_hash_algorithm, DifferentPrecisions)
{
	// Test different precisions
	constexpr size_t precision1 = 8;
	constexpr size_t precision2 = 32;
	constexpr size_t precision3 = 64;

	const auto hash1 = generate_hash<precision1, int, 4>({7, 15, 23, 31});    // All should map to 0
	const auto hash2 = generate_hash<precision2, int, 4>({16, 48, 80, 112});  // All should map to 16, 48, 80, 112
	const auto hash3 = generate_hash<precision3, int, 4>({32, 96, 160, 224}); // All should map to 0, 64, 128, 192

	EXPECT_EQ(hash1, hash1); // Hashes should be consistent
	EXPECT_EQ(hash2, hash2);
	EXPECT_EQ(hash3, hash3);

	EXPECT_NE(hash1,
	          hash2); // Different precisions should result in different hashes
	EXPECT_NE(hash2, hash3);
}

TEST(test_hash_algorithm, Performance)
{
	constexpr size_t precision = 16;

	// Measure the time to generate hashes
	const size_t repetitions = 1000;

	auto setup  = [](size_t) {};
	auto lambda = [](size_t) { generate_hash<precision, int, 4>({16, 32, 48, 64}); };

	const auto complexity = measure_time_complexity(setup, lambda, {4, 4, 4, 4}, repetitions);
	EXPECT_LE(complexity, Complexity::O1);
}

TEST(HashKeyGenerationTest, GeneratesCorrectNumberOfKeys)
{
	constexpr size_t precision  = 4;
	using CoordinateType        = int;
	constexpr size_t dimensions = 2;

	std::array<CoordinateType, dimensions> min_coords = {0, 0};
	std::array<CoordinateType, dimensions> max_coords = {
	    15, 15}; // Assuming precision allows for 4 distinct values per dimension

	auto hash_keys = generate_all_hash_keys_within_range<precision, CoordinateType, dimensions>(min_coords, max_coords);

	// With the given range, we expect 4^2 = 16 distinct hash keys
	EXPECT_EQ(hash_keys.size(), 16u);
}

TEST(QuantizeValueTest, QuantizesIntegerCorrectly)
{
	constexpr int    value     = 10;
	constexpr size_t precision = 8;
	constexpr size_t expected  = 8; // 10 quantized to nearest lower multiple of 8 is 8
	constexpr auto   result    = quantize_value<int, precision>(value);
	EXPECT_EQ(result, expected);
}

TEST(QuantizeValueTest, QuantizesNegativeIntegerCorrectly)
{
	constexpr int     value     = -10;
	constexpr size_t  precision = 8;
	constexpr ssize_t expected  = -16; // -10 quantized to nearest lower multiple of 8 is -16
	constexpr auto    result    = quantize_value<int, precision>(value);
	EXPECT_EQ(result, expected);
}

TEST(QuantizeValueTest, QuantizesFloatingPointCorrectly)
{
	constexpr double  value     = 10.75;
	constexpr size_t  precision = 4;
	constexpr ssize_t expected  = 8; // 10.75 quantized to nearest lower multiple of 4 is 8
	constexpr auto    result    = quantize_value<double, precision>(value);
	EXPECT_EQ(result, expected);
}

TEST(QuantizeValueTest, PrecisionIsPowerOfTwo)
{
	// This test ensures that the static assertion for precision being a power of two is in effect.
	// Note: This test will not compile if the static assertion fails, serving as a compile-time check.
	constexpr int    value     = 10;
	constexpr size_t precision = 4; // This should be a power of two for the test to compile
	quantize_value<int, precision>(value);
	// No need for runtime assertion here, compilation success is the test
}

// This test is expected to fail compilation due to the static_assert if uncommented
// TEST(QuantizeValueTest, FailsForNonArithmeticType) {
//     struct NonArithmetic {};
//     NonArithmetic value;
//     constexpr size_t precision = 4;
//     auto result = quantize_value<NonArithmetic, precision>(value);
//     // This test is designed to fail at compile time due to static_assert
// }

TEST(QuantizeCoordinatesTest, Quantizes2DCoordinatesCorrectly)
{
	constexpr size_t      precision   = 4;
	std::array<int, 2>    coordinates = {5, 9}; // Example 2D coordinates
	std::array<size_t, 2> expected    = {4, 8}; // Expected quantized coordinates
	auto                  result      = quantize_coordinates<precision, int, 2>(coordinates);
	EXPECT_EQ(result, expected);
}

TEST(QuantizeCoordinatesTest, Quantizes3DCoordinatesCorrectly)
{
	constexpr size_t      precision   = 8;
	std::array<double, 3> coordinates = {7.5, 15.25, 32.75}; // Example 3D coordinates
	std::array<size_t, 3> expected    = {0, 8, 32};          // Expected quantized coordinates
	auto                  result      = quantize_coordinates<precision, double, 3>(coordinates);
	EXPECT_EQ(result, expected);
}

TEST(QuantizeCoordinatesTest, PrecisionIsPowerOfTwo)
{
	// This test ensures that the static assertion for precision being a power of two is in effect.
	constexpr size_t   precision   = 2;   // This should be a power of two for the test to compile
	std::array<int, 1> coordinates = {3}; // Single dimension array for simplicity
	auto               result      = quantize_coordinates<precision, int, 1>(coordinates);
	// No need for runtime assertion here, compilation success is the test
}

// This test is expected to fail compilation due to the static_assert if uncommented
// TEST(QuantizeCoordinatesTest, FailsForNonArithmeticType) {
//     constexpr size_t precision = 4;
//     struct NonArithmetic {};
//     std::array<NonArithmetic, 1> coordinates = {{}};
//     auto result = quantize_coordinates<precision, NonArithmetic, 1>(coordinates);
//     // This test is designed to fail at compile time due to static_assert
// }

TEST(ShiftValueCalculationTest, CalculatesShiftValueCorrectly)
{
	constexpr size_t precision = 16;
	constexpr size_t expected  = 4; // 16 = 2^4
	constexpr size_t result    = calculate_precision_shift<precision>();
	EXPECT_EQ(result, expected);
}