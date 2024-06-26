#include "lochash/lochash.hpp"
#include "test_helpers.hpp"
#include "gtest/gtest.h"
#include <chrono>

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

// Test relative performmance for generate_hash based on the number of floating point arguments at 2, 20 and 200 to
// determine if the complexity is O(1), O(n), O(log(n)), or O(n^2) or worse.
TEST(test_hash_algorithm, PerformanceParameterCount)
{
	constexpr std::size_t precision = 16;

	// Test with 2 parameters
	float x           = 3.14f;
	float y           = 1.59f;
	auto  start       = std::chrono::high_resolution_clock::now();
	auto  hash_value1 = generate_hash<precision>(x, y);
	auto  end         = std::chrono::high_resolution_clock::now();
	auto  duration1   = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Test with 20 parameters
	float a1 = 0.1f, a2 = 0.2f, a3 = 0.3f, a4 = 0.4f, a5 = 0.5f;
	float a6 = 0.6f, a7 = 0.7f, a8 = 0.8f, a9 = 0.9f, a10 = 1.0f;
	float a11 = 1.1f, a12 = 1.2f, a13 = 1.3f, a14 = 1.4f, a15 = 1.5f;
	float a16 = 1.6f, a17 = 1.7f, a18 = 1.8f, a19 = 1.9f, a20 = 2.0f;
	start            = std::chrono::high_resolution_clock::now();
	auto hash_value2 = generate_hash<precision>(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16,
	                                            a17, a18, a19, a20);
	end              = std::chrono::high_resolution_clock::now();
	auto duration2   = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Test with 200 parameters
	float b1 = 0.1f, b2 = 0.2f, b3 = 0.3f, b4 = 0.4f, b5 = 0.5f;
	float b6 = 0.6f, b7 = 0.7f, b8 = 0.8f, b9 = 0.9f, b10 = 1.0f;
	float b11 = 1.1f, b12 = 1.2f, b13 = 1.3f, b14 = 1.4f, b15 = 1.5f;
	float b16 = 1.6f, b17 = 1.7f, b18 = 1.8f, b19 = 1.9f, b20 = 2.0f;
	float b21 = 2.1f, b22 = 2.2f, b23 = 2.3f, b24 = 2.4f, b25 = 2.5f;
	float b26 = 2.6f, b27 = 2.7f, b28 = 2.8f, b29 = 2.9f, b30 = 3.0f;
	float b31 = 3.1f, b32 = 3.2f, b33 = 3.3f, b34 = 3.4f, b35 = 3.5f;
	float b36 = 3.6f, b37 = 3.7f, b38 = 3.8f, b39 = 3.9f, b40 = 4.0f;
	float b41 = 4.1f, b42 = 4.2f, b43 = 4.3f, b44 = 4.4f, b45 = 4.5f;
	float b46 = 4.6f, b47 = 4.7f, b48 = 4.8f, b49 = 4.9f, b50 = 5.0f;
	float b51 = 5.1f, b52 = 5.2f, b53 = 5.3f, b54 = 5.4f, b55 = 5.5f;
	float b56 = 5.6f, b57 = 5.7f, b58 = 5.8f, b59 = 5.9f, b60 = 6.0f;
	float b61 = 6.1f, b62 = 6.2f, b63 = 6.3f, b64 = 6.4f, b65 = 6.5f;
	float b66 = 6.6f, b67 = 6.7f, b68 = 6.8f, b69 = 6.9f, b70 = 7.0f;
	float b71 = 7.1f, b72 = 7.2f, b73 = 7.3f, b74 = 7.4f, b75 = 7.5f;
	float b76 = 7.6f, b77 = 7.7f, b78 = 7.8f, b79 = 7.9f, b80 = 8.0f;
	float b81 = 8.1f, b82 = 8.2f, b83 = 8.3f, b84 = 8.4f, b85 = 8.5f;
	float b86 = 8.6f, b87 = 8.7f, b88 = 8.8f, b89 = 8.9f, b90 = 9.0f;
	float b91 = 9.1f, b92 = 9.2f, b93 = 9.3f, b94 = 9.4f, b95 = 9.5f;
	float b96 = 9.6f, b97 = 9.7f, b98 = 9.8f, b99 = 9.9f, b100 = 10.0f;
	float b101 = 10.1f, b102 = 10.2f, b103 = 10.3f, b104 = 10.4f, b105 = 10.5f;
	float b106 = 10.6f, b107 = 10.7f, b108 = 10.8f, b109 = 10.9f, b110 = 11.0f;
	float b111 = 11.1f, b112 = 11.2f, b113 = 11.3f, b114 = 11.4f, b115 = 11.5f;
	float b116 = 11.6f, b117 = 11.7f, b118 = 11.8f, b119 = 11.9f, b120 = 12.0f;
	float b121 = 12.1f, b122 = 12.2f, b123 = 12.3f, b124 = 12.4f, b125 = 12.5f;
	float b126 = 12.6f, b127 = 12.7f, b128 = 12.8f, b129 = 12.9f, b130 = 13.0f;
	float b131 = 13.1f, b132 = 13.2f, b133 = 13.3f, b134 = 13.4f, b135 = 13.5f;
	float b136 = 13.6f, b137 = 13.7f, b138 = 13.8f, b139 = 13.9f, b140 = 14.0f;
	float b141 = 14.1f, b142 = 14.2f, b143 = 14.3f, b144 = 14.4f, b145 = 14.5f;
	float b146 = 14.6f, b147 = 14.7f, b148 = 14.8f, b149 = 14.9f, b150 = 15.0f;
	float b151 = 15.1f, b152 = 15.2f, b153 = 15.3f, b154 = 15.4f, b155 = 15.5f;
	float b156 = 15.6f, b157 = 15.7f, b158 = 15.8f, b159 = 15.9f, b160 = 16.0f;
	float b161 = 16.1f, b162 = 16.2f, b163 = 16.3f, b164 = 16.4f, b165 = 16.5f;
	float b166 = 16.6f, b167 = 16.7f, b168 = 16.8f, b169 = 16.9f, b170 = 17.0f;
	float b171 = 17.1f, b172 = 17.2f, b173 = 17.3f, b174 = 17.4f, b175 = 17.5f;
	float b176 = 17.6f, b177 = 17.7f, b178 = 17.8f, b179 = 17.9f, b180 = 18.0f;
	float b181 = 18.1f, b182 = 18.2f, b183 = 18.3f, b184 = 18.4f, b185 = 18.5f;
	float b186 = 18.6f, b187 = 18.7f, b188 = 18.8f, b189 = 18.9f, b190 = 19.0f;
	float b191 = 19.1f, b192 = 19.2f, b193 = 19.3f, b194 = 19.4f, b195 = 19.5f;
	float b196 = 19.6f, b197 = 19.7f, b198 = 19.8f, b199 = 19.9f, b200 = 20.0f;

	start            = std::chrono::high_resolution_clock::now();
	auto hash_value3 = generate_hash<precision>(
	    b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24,
	    b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, b36, b37, b38, b39, b40, b41, b42, b43, b44, b45, b46,
	    b47, b48, b49, b50, b51, b52, b53, b54, b55, b56, b57, b58, b59, b60, b61, b62, b63, b64, b65, b66, b67, b68,
	    b69, b70, b71, b72, b73, b74, b75, b76, b77, b78, b79, b80, b81, b82, b83, b84, b85, b86, b87, b88, b89, b90,
	    b91, b92, b93, b94, b95, b96, b97, b98, b99, b100, b101, b102, b103, b104, b105, b106, b107, b108, b109, b110,
	    b111, b112, b113, b114, b115, b116, b117, b118, b119, b120, b121, b122, b123, b124, b125, b126, b127, b128,
	    b129, b130, b131, b132, b133, b134, b135, b136, b137, b138, b139, b140, b141, b142, b143, b144, b145, b146,
	    b147, b148, b149, b150, b151, b152, b153, b154, b155, b156, b157, b158, b159, b160, b161, b162, b163, b164,
	    b165, b166, b167, b168, b169, b170, b171, b172, b173, b174, b175, b176, b177, b178, b179, b180, b181, b182,
	    b183, b184, b185, b186, b187, b188, b189, b190, b191, b192, b193, b194, b195, b196, b197, b198, b199, b200);
	end            = std::chrono::high_resolution_clock::now();
	auto duration3 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

	// Complexity must be O(n) for the number of parameters
	// Change to O1 to verify it fails since the algorithm is not O1 complexity
	SCOPED_TRACE("Testing performance based on parameter count. Should be about O(n)");
	test_complexity({2, 20, 200}, {duration1, duration2, duration3}, ComplexityThreshold::ON);
}