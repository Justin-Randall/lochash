#include "lochash/lochash.hpp"
#include "test_helpers.hpp"
#include "gtest/gtest.h"
#include <chrono>

using namespace lochash;

TEST(test_hash_algorithm, IntegerHashing)
{
	constexpr std::size_t precision = 16;

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
	constexpr std::size_t precision = 16;

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
	constexpr std::size_t precision = 16;

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
	constexpr std::size_t precision = 16;

	const auto int_hash    = generate_hash<precision, int, 4>({16, 32, 48, 64});
	const auto float_hash  = generate_hash<precision, float, 4>({16.0f, 32.0f, 48.0f, 64.0f});
	const auto double_hash = generate_hash<precision, double, 4>({16.0, 32.0, 48.0, 64.0});

	// All values should hash to the same bucket
	EXPECT_EQ(int_hash, float_hash);
	EXPECT_EQ(float_hash, double_hash);
}

TEST(test_hash_algorithm, EdgeCases)
{
	constexpr std::size_t precision = 16;

	// Test with zero values
	static const auto zero_hash = generate_hash<precision, int, 4>({0, 0, 0, 0});
	EXPECT_EQ(zero_hash, zero_hash); // Should always be equal to itself

	// Test with maximum precision values
	const auto max_precision_hash =
	    generate_hash<precision, size_t, 4>({precision - 1, precision - 1, precision - 1, precision - 1});
	EXPECT_EQ(max_precision_hash, max_precision_hash); // Should always be equal to itself

	// Test with mixed zero and precision values
	const auto mixed_hash1 =
	    generate_hash<precision, size_t, 4>({std::size_t(0), precision, 2 * precision, 3 * precision});
	const auto mixed_hash2 =
	    generate_hash<precision, size_t, 4>({std::size_t(0), precision, 2 * precision, 3 * precision});

	EXPECT_EQ(mixed_hash1, mixed_hash2); // Should be equal
}

TEST(test_hash_algorithm, DifferentPrecisions)
{
	// Test different precisions
	constexpr std::size_t precision1 = 8;
	constexpr std::size_t precision2 = 32;
	constexpr std::size_t precision3 = 64;

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

// Test relative performmance for generate_hash based on the number of floating point arguments at 2, 20 and 200 to
// determine if the complexity is O(1), O(n), O(log(n)), or O(n^2) or worse.
TEST(test_hash_algorithm, PerformanceParameterCount)
{
	constexpr std::size_t precision = 16;

	// Test with 2 parameters
	const std::array<float, 2> params2     = {3.14f, 1.59f};
	const auto                 start1      = std::chrono::high_resolution_clock::now();
	const auto                 hash_value1 = generate_hash<precision>(params2);
	const auto                 end1        = std::chrono::high_resolution_clock::now();
	const auto                 duration1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1).count();

	// Test with 20 parameters
	const std::array<float, 20> params20    = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f,
                                            1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f, 2.0f};
	const auto                  start2      = std::chrono::high_resolution_clock::now();
	const auto                  hash_value2 = generate_hash<precision>(params20);
	const auto                  end2        = std::chrono::high_resolution_clock::now();
	const auto                  duration2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - start2).count();

	// Test with 200 parameters
	const std::array<float, 200> params200 = {
	    0.1f,  0.2f,  0.3f,  0.4f,  0.5f,  0.6f,  0.7f,  0.8f,  0.9f,  1.0f,  1.1f,  1.2f,  1.3f,  1.4f,  1.5f,  1.6f,
	    1.7f,  1.8f,  1.9f,  2.0f,  2.1f,  2.2f,  2.3f,  2.4f,  2.5f,  2.6f,  2.7f,  2.8f,  2.9f,  3.0f,  3.1f,  3.2f,
	    3.3f,  3.4f,  3.5f,  3.6f,  3.7f,  3.8f,  3.9f,  4.0f,  4.1f,  4.2f,  4.3f,  4.4f,  4.5f,  4.6f,  4.7f,  4.8f,
	    4.9f,  5.0f,  5.1f,  5.2f,  5.3f,  5.4f,  5.5f,  5.6f,  5.7f,  5.8f,  5.9f,  6.0f,  6.1f,  6.2f,  6.3f,  6.4f,
	    6.5f,  6.6f,  6.7f,  6.8f,  6.9f,  7.0f,  7.1f,  7.2f,  7.3f,  7.4f,  7.5f,  7.6f,  7.7f,  7.8f,  7.9f,  8.0f,
	    8.1f,  8.2f,  8.3f,  8.4f,  8.5f,  8.6f,  8.7f,  8.8f,  8.9f,  9.0f,  9.1f,  9.2f,  9.3f,  9.4f,  9.5f,  9.6f,
	    9.7f,  9.8f,  9.9f,  10.0f, 10.1f, 10.2f, 10.3f, 10.4f, 10.5f, 10.6f, 10.7f, 10.8f, 10.9f, 11.0f, 11.1f, 11.2f,
	    11.3f, 11.4f, 11.5f, 11.6f, 11.7f, 11.8f, 11.9f, 12.0f, 12.1f, 12.2f, 12.3f, 12.4f, 12.5f, 12.6f, 12.7f, 12.8f,
	    12.9f, 13.0f, 13.1f, 13.2f, 13.3f, 13.4f, 13.5f, 13.6f, 13.7f, 13.8f, 13.9f, 14.0f, 14.1f, 14.2f, 14.3f, 14.4f,
	    14.5f, 14.6f, 14.7f, 14.8f, 14.9f, 15.0f, 15.1f, 15.2f, 15.3f, 15.4f, 15.5f, 15.6f, 15.7f, 15.8f, 15.9f, 16.0f,
	    16.1f, 16.2f, 16.3f, 16.4f, 16.5f, 16.6f, 16.7f, 16.8f, 16.9f, 17.0f, 17.1f, 17.2f, 17.3f, 17.4f, 17.5f, 17.6f,
	    17.7f, 17.8f, 17.9f, 18.0f, 18.1f, 18.2f, 18.3f, 18.4f, 18.5f, 18.6f, 18.7f, 18.8f, 18.9f, 19.0f, 19.1f, 19.2f,
	    19.3f, 19.4f, 19.5f, 19.6f, 19.7f, 19.8f, 19.9f, 20.0f};
	const auto start3      = std::chrono::high_resolution_clock::now();
	const auto hash_value3 = generate_hash<precision>(params200);
	const auto end3        = std::chrono::high_resolution_clock::now();
	const auto duration3   = std::chrono::duration_cast<std::chrono::nanoseconds>(end3 - start3).count();

	SCOPED_TRACE("Testing performance based on parameter count. Should be about O(n)");
	const std::vector<size_t>    counts{2, 20, 200};
	const std::vector<long long> durations{duration1, duration2, duration3};
	const auto                   determined_complexity = test_complexity(counts, durations);
	EXPECT_LE(determined_complexity, ComplexityThreshold::ON);
}
