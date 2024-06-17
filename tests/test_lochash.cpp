#include "lochash/lochash.hpp"
#include "gtest/gtest.h"

// Define a custom type for testing with associated objects
struct TestObject {
	int         id;
	std::string name;
};

TEST(LochashTest, IntegerHashing)
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

TEST(LochashTest, FloatHashing)
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

TEST(LochashTest, DoubleHashing)
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

TEST(LochashTest, MixedTypesHashing)
{
	constexpr std::size_t precision = 16;

	auto int_hash    = generate_hash<precision>(16, 32, 48, 64);
	auto float_hash  = generate_hash<precision>(16.0f, 32.0f, 48.0f, 64.0f);
	auto double_hash = generate_hash<precision>(16.0, 32.0, 48.0, 64.0);

	// All values should hash to the same bucket
	EXPECT_EQ(int_hash, float_hash);
	EXPECT_EQ(float_hash, double_hash);
}

TEST(LochashTest, EdgeCases)
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

TEST(LochashTest, DifferentPrecisions)
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

// ----------------------------------------------------------------------------
// Location Hash Testing
// Test moving items in LocationHash with 2D coordinates
TEST(LocationHashTest, Move2DCoordinates)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 2D coordinates with no associated object
	LocationHash<precision, float> locationHash;

	// Add some 2D coordinates
	locationHash.add(1.0f, 2.0f);
	locationHash.add(16.0f, 32.0f);

	// Move an existing coordinate
	EXPECT_TRUE(locationHash.move(std::make_tuple(1.0f, 2.0f), std::make_tuple(10.0f, 20.0f)));

	// Ensure the coordinate is moved
	const auto & old_bucket = locationHash.query(1.0f, 2.0f);
	EXPECT_TRUE(old_bucket.empty());

	const auto & new_bucket = locationHash.query(10.0f, 20.0f);
	ASSERT_EQ(new_bucket.size(), 1);
	EXPECT_EQ(new_bucket[0].first, (LocationHash<precision, float>::CoordinateVector{10.0f, 20.0f}));

	// Move a non-existing coordinate
	EXPECT_FALSE(locationHash.move(std::make_tuple(100.0f, 200.0f), std::make_tuple(150.0f, 250.0f)));
}
// Test moving items in LocationHash with 3D coordinates and associated object
TEST(LocationHashTest, Move3DCoordinatesWithObject)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 3D coordinates with associated TestObject
	LocationHash<precision, double, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, 1.0, 2.0, 3.0);
	locationHash.add(&obj2, 16.0, 32.0, 48.0);

	// Move an existing coordinate by object
	EXPECT_TRUE(locationHash.move(&obj1, std::make_tuple(1.0, 2.0, 3.0), std::make_tuple(10.0, 20.0, 30.0)));

	// Ensure the coordinate is moved
	const auto & old_bucket = locationHash.query(1.0, 2.0, 3.0);
	EXPECT_TRUE(old_bucket.empty());

	const auto & new_bucket = locationHash.query(10.0, 20.0, 30.0);
	ASSERT_EQ(new_bucket.size(), 1);
	EXPECT_EQ(new_bucket[0].second, &obj1);

	// Move a non-existing coordinate by object
	EXPECT_FALSE(locationHash.move(&obj1, std::make_tuple(100.0, 200.0, 300.0), std::make_tuple(150.0, 250.0, 350.0)));
}

// Test moving items in LocationHash with 4D coordinates using epsilon for floating-point types
TEST(LocationHashTest, Move4DCoordinatesWithEpsilon)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 4D coordinates with no associated object
	LocationHash<precision, float> locationHash;

	// Add some 4D coordinates
	locationHash.add(1.0f, 2.0f, 3.0f, 4.0f);
	locationHash.add(16.0f, 32.0f, 48.0f, 64.0f);

	// Move an existing coordinate with epsilon comparison
	EXPECT_TRUE(
	    locationHash.move(std::make_tuple(1.0f, 2.0f, 3.0f, 4.0f), std::make_tuple(10.0f, 20.0f, 30.0f, 40.0f)));

	// Ensure the coordinate is moved
	const auto & old_bucket = locationHash.query(1.0f, 2.0f, 3.0f, 4.0f);
	EXPECT_TRUE(old_bucket.empty());

	const auto & new_bucket = locationHash.query(10.0f, 20.0f, 30.0f, 40.0f);
	ASSERT_EQ(new_bucket.size(), 1);
	EXPECT_EQ(new_bucket[0].first, (LocationHash<precision, float>::CoordinateVector{10.0f, 20.0f, 30.0f, 40.0f}));

	// Move a non-existing coordinate
	EXPECT_FALSE(locationHash.move(std::make_tuple(100.0f, 200.0f, 300.0f, 400.0f),
	                               std::make_tuple(150.0f, 250.0f, 350.0f, 450.0f)));
}

// Test adding, querying, and removing items in LocationHash with 2D coordinates
TEST(LocationHashTest, AddQueryRemove2DCoordinates)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 2D coordinates with no associated object
	LocationHash<precision, float> locationHash;

	// Add some 2D coordinates
	locationHash.add(1.0f, 2.0f);
	locationHash.add(16.0f, 32.0f);

	// Query and ensure coordinates are present
	const auto & bucket1 = locationHash.query(1.0f, 2.0f);
	ASSERT_EQ(bucket1.size(), 1);
	EXPECT_EQ(bucket1[0].first, (LocationHash<precision, float>::CoordinateVector{1.0f, 2.0f}));

	const auto & bucket2 = locationHash.query(16.0f, 32.0f);
	ASSERT_EQ(bucket2.size(), 1);
	EXPECT_EQ(bucket2[0].first, (LocationHash<precision, float>::CoordinateVector{16.0f, 32.0f}));

	// Remove and ensure coordinates are removed
	EXPECT_TRUE(locationHash.remove(1.0f, 2.0f));
	EXPECT_TRUE(locationHash.query(1.0f, 2.0f).empty());

	EXPECT_TRUE(locationHash.remove(16.0f, 32.0f));
	EXPECT_TRUE(locationHash.query(16.0f, 32.0f).empty());
}

// Test adding, querying, and removing items in LocationHash with 3D coordinates and associated object
TEST(LocationHashTest, AddQueryRemove3DCoordinatesWithObject)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 3D coordinates with associated TestObject
	LocationHash<precision, double, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, 1.0, 2.0, 3.0);
	locationHash.add(&obj2, 16.0, 32.0, 48.0);

	// Query and ensure coordinates and objects are present
	const auto & bucket1 = locationHash.query(1.0, 2.0, 3.0);
	ASSERT_EQ(bucket1.size(), 1);
	EXPECT_EQ(bucket1[0].second, &obj1);

	const auto & bucket2 = locationHash.query(16.0, 32.0, 48.0);
	ASSERT_EQ(bucket2.size(), 1);
	EXPECT_EQ(bucket2[0].second, &obj2);

	// Remove and ensure coordinates and objects are removed
	EXPECT_TRUE(locationHash.remove(&obj1, 1.0, 2.0, 3.0));
	EXPECT_TRUE(locationHash.query(1.0, 2.0, 3.0).empty());

	EXPECT_TRUE(locationHash.remove(&obj2, 16.0, 32.0, 48.0));
	EXPECT_TRUE(locationHash.query(16.0, 32.0, 48.0).empty());
}

// Test adding, querying, and removing items in LocationHash with 4D coordinates using epsilon for floating-point types
TEST(LocationHashTest, AddQueryRemove4DCoordinatesWithEpsilon)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 4D coordinates with no associated object
	LocationHash<precision, float> locationHash;

	// Add some 4D coordinates
	locationHash.add(1.0f, 2.0f, 3.0f, 4.0f);
	locationHash.add(16.0f, 32.0f, 48.0f, 64.0f);

	// Query and ensure coordinates are present
	const auto & bucket1 = locationHash.query(1.0f, 2.0f, 3.0f, 4.0f);
	ASSERT_EQ(bucket1.size(), 1);
	EXPECT_EQ(bucket1[0].first, (LocationHash<precision, float>::CoordinateVector{1.0f, 2.0f, 3.0f, 4.0f}));

	const auto & bucket2 = locationHash.query(16.0f, 32.0f, 48.0f, 64.0f);
	ASSERT_EQ(bucket2.size(), 1);
	EXPECT_EQ(bucket2[0].first, (LocationHash<precision, float>::CoordinateVector{16.0f, 32.0f, 48.0f, 64.0f}));

	// Remove and ensure coordinates are removed
	EXPECT_TRUE(locationHash.remove(1.0f, 2.0f, 3.0f, 4.0f));
	EXPECT_TRUE(locationHash.query(1.0f, 2.0f, 3.0f, 4.0f).empty());

	EXPECT_TRUE(locationHash.remove(16.0f, 32.0f, 48.0f, 64.0f));
	EXPECT_TRUE(locationHash.query(16.0f, 32.0f, 48.0f, 64.0f).empty());
}
// ----------------------------------------------------------------------------
