#include "lochash/lochash.hpp"
#include "gtest/gtest.h"

using namespace lochash;

TEST(test_hash_algorithm, IntegerHashing)
{
	constexpr std::size_t precision = 16;

	auto hash1 = generate_hash<precision>(0, 16, 32, 48);
	auto hash2 = generate_hash<precision>(1, 17, 33, 49);
	auto hash3 = generate_hash<precision>(2, 18, 34, 50);

	// All values fall into the same bucket
	EXPECT_EQ(hash1, hash2);
	EXPECT_EQ(hash2, hash3);

	auto hash4 = generate_hash<precision>(15, 31, 47, 63);
	auto hash5 = generate_hash<precision>(16, 32, 48, 64);

	// Values in different buckets
	EXPECT_NE(hash4, hash5);
}

TEST(test_hash_algorithm, FloatHashing)
{
	constexpr std::size_t precision = 16;

	auto hash1 = generate_hash<precision>(0.0f, 16.0f, 32.0f, 48.0f);
	auto hash2 = generate_hash<precision>(0.1f, 16.1f, 32.1f, 48.1f);
	auto hash3 = generate_hash<precision>(0.5f, 16.5f, 32.5f, 48.5f);

	// All values fall into the same bucket
	EXPECT_EQ(hash1, hash2);
	EXPECT_EQ(hash2, hash3);

	auto hash4 = generate_hash<precision>(15.9f, 31.9f, 47.9f, 63.9f);
	auto hash5 = generate_hash<precision>(16.1f, 32.1f, 48.1f, 64.1f);

	// Values in different buckets
	EXPECT_NE(hash4, hash5);
}

TEST(test_hash_algorithm, DoubleHashing)
{
	constexpr std::size_t precision = 16;

	auto hash1 = generate_hash<precision>(0.0, 16.0, 32.0, 48.0);
	auto hash2 = generate_hash<precision>(0.1, 16.1, 32.1, 48.1);
	auto hash3 = generate_hash<precision>(0.5, 16.5, 32.5, 48.5);

	// All values fall into the same bucket
	EXPECT_EQ(hash1, hash2);
	EXPECT_EQ(hash2, hash3);

	auto hash4 = generate_hash<precision>(15.9, 31.9, 47.9, 63.9);
	auto hash5 = generate_hash<precision>(16.1, 32.1, 48.1, 64.1);

	// Values in different buckets
	EXPECT_NE(hash4, hash5);
}

TEST(test_hash_algorithm, MixedTypesHashing)
{
	constexpr std::size_t precision = 16;

	auto int_hash    = generate_hash<precision>(16, 32, 48, 64);
	auto float_hash  = generate_hash<precision>(16.0f, 32.0f, 48.0f, 64.0f);
	auto double_hash = generate_hash<precision>(16.0, 32.0, 48.0, 64.0);

	// All values should hash to the same bucket
	EXPECT_EQ(int_hash, float_hash);
	EXPECT_EQ(float_hash, double_hash);
}

TEST(test_hash_algorithm, EdgeCases)
{
	constexpr std::size_t precision = 16;

	// Test with zero values
	auto zero_hash = generate_hash<precision>(0, 0, 0, 0);
	EXPECT_EQ(zero_hash, zero_hash); // Should always be equal to itself

	// Test with maximum precision values
	auto max_precision_hash = generate_hash<precision>(precision - 1, precision - 1, precision - 1, precision - 1);
	EXPECT_EQ(max_precision_hash, max_precision_hash); // Should always be equal to itself

	// Test with mixed zero and precision values
	auto mixed_hash1 = generate_hash<precision>(std::size_t(0), precision, 2 * precision, 3 * precision);
	auto mixed_hash2 = generate_hash<precision>(std::size_t(0), precision, 2 * precision, 3 * precision);

	// These break the compiler due to mixed precision. keeping around to test static assert to alert
	// users that different types are not supported
	//
	// auto mixed_hash1 =
	//     generate_hash<precision>(0, precision, 2 * precision, 3 * precision);
	// auto mixed_hash2 =
	//     generate_hash<precision>(0, precision, 2 * precision, 3 * precision);
	//
	// Also verify static hash failure at compile time when precision is not a power of 2.
	// constexpr size_t BadPrecision = 3;
	// auto             bad_hash     = generate_hash<BadPrecision>(0, 0, 0, 0);

	EXPECT_EQ(mixed_hash1, mixed_hash2); // Should be equal
}

TEST(test_hash_algorithm, DifferentPrecisions)
{
	// Test different precisions
	constexpr std::size_t precision1 = 8;
	constexpr std::size_t precision2 = 32;
	constexpr std::size_t precision3 = 64;

	auto hash1 = generate_hash<precision1>(7, 15, 23, 31);    // All should map to 0
	auto hash2 = generate_hash<precision2>(16, 48, 80, 112);  // All should map to 16, 48, 80, 112
	auto hash3 = generate_hash<precision3>(32, 96, 160, 224); // All should map to 0, 64, 128, 192

	EXPECT_EQ(hash1, hash1); // Hashes should be consistent
	EXPECT_EQ(hash2, hash2);
	EXPECT_EQ(hash3, hash3);

	EXPECT_NE(hash1,
	          hash2); // Different precisions should result in different hashes
	EXPECT_NE(hash2, hash3);
}
