#include "lochash/lochash.hpp"
#include "test_helpers.hpp"
#include "gtest/gtest.h"
#include <chrono>

using namespace lochash;

TEST(HashKeyGenerationTest, GeneratesCorrectNumberOfKeys)
{
	constexpr size_t precision  = 4;
	using CoordinateType        = int;
	constexpr size_t dimensions = 2;

	std::array<CoordinateType, dimensions> min_coords = {0, 0};
	std::array<CoordinateType, dimensions> max_coords = {
	    15, 15}; // Assuming precision allows for 4 distinct values per dimension

	auto hash_keys =
	    generate_all_quantized_coordinates_within_range<precision, CoordinateType, dimensions>(min_coords, max_coords);

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

TEST(ShiftValueCalculationTest, CalculatesShiftValueCorrectly)
{
	constexpr size_t precision = 16;
	constexpr size_t expected  = 4; // 16 = 2^4
	constexpr size_t result    = calculate_precision_shift<precision>();
	EXPECT_EQ(result, expected);
}
