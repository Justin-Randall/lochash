#include "lochash/location_hash_query_bounding_box.hpp"
#include "lochash/lochash.hpp"
#include "gtest/gtest.h"

using namespace lochash;

// Define a custom type for testing with associated objects
struct TestObject {
	int         id;
	std::string name;
};

// ----------------------------------------------------------------------------
// Location Hash Testing
// Test moving items in LocationHash with 2D coordinates
TEST(LocationHashTest, Move2DCoordinates)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 2D coordinates with no associated object
	LocationHash<precision, float, 2> locationHash;

	// Add some 2D coordinates
	locationHash.add({1.0f, 2.0f});
	locationHash.add({16.0f, 32.0f});

	// Move an existing coordinate
	EXPECT_TRUE(locationHash.move({1.0f, 2.0f}, {10.0f, 20.0f}));

	// The quantized coordinates should be the same and so should the hash key.
	// The move() operation should return false (not moved) if the coordinates are the same.
	EXPECT_FALSE(locationHash.move({10.0f, 20.0f}, {11.0f, 21.0f}));

	// Ensure the coordinate is moved
	const auto & old_bucket = locationHash.query({1.0f, 2.0f});
	EXPECT_TRUE(old_bucket.empty());

	const auto & new_bucket = locationHash.query({10.0f, 20.0f});
	ASSERT_EQ(new_bucket.size(), 1);
	EXPECT_EQ(new_bucket[0].first, (LocationHash<precision, float, 2>::CoordinateArray{10.0f, 20.0f}));

	// Move a non-existing coordinate
	EXPECT_FALSE(locationHash.move({100.0f, 200.0f}, {150.0f, 250.0f}));
}
// Test moving items in LocationHash with 3D coordinates and associated object
TEST(LocationHashTest, Move3DCoordinatesWithObject)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 3D coordinates with associated TestObject
	LocationHash<precision, double, 3, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, {1.0, 2.0, 3.0});
	locationHash.add(&obj2, {16.0, 32.0, 48.0});

	// Move an existing coordinate by object
	EXPECT_TRUE(locationHash.move(&obj1, {1.0, 2.0, 3.0}, {10.0, 20.0, 30.0}));

	// Ensure the coordinate is moved
	const auto & old_bucket = locationHash.query({1.0, 2.0, 3.0});
	EXPECT_TRUE(old_bucket.empty());

	const auto & new_bucket = locationHash.query({10.0, 20.0, 30.0});
	ASSERT_EQ(new_bucket.size(), 1);
	EXPECT_EQ(new_bucket[0].second, &obj1);

	// Move a non-existing coordinate by object
	EXPECT_FALSE(locationHash.move(&obj1, {100.0, 200.0, 300.0}, {150.0, 250.0, 350.0}));

	// Move an existing object within the precision parameters and expect a false result
	EXPECT_FALSE(locationHash.move(&obj1, {10.0, 20.0, 30.0}, {10.1, 20.1, 30.1}));
}

// Test moving items in LocationHash with 4D coordinates using epsilon for floating-point types
TEST(LocationHashTest, Move4DCoordinatesWithEpsilon)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 4D coordinates with no associated object
	LocationHash<precision, float, 4> locationHash;

	// Add some 4D coordinates
	locationHash.add({1.0f, 2.0f, 3.0f, 4.0f});
	locationHash.add({16.0f, 32.0f, 48.0f, 64.0f});

	// Move an existing coordinate with epsilon comparison
	EXPECT_TRUE(locationHash.move({1.0f, 2.0f, 3.0f, 4.0f}, {10.0f, 20.0f, 30.0f, 40.0f}));

	// Ensure the coordinate is moved
	const auto & old_bucket = locationHash.query({1.0f, 2.0f, 3.0f, 4.0f});
	EXPECT_TRUE(old_bucket.empty());

	const auto & new_bucket = locationHash.query({10.0f, 20.0f, 30.0f, 40.0f});
	ASSERT_EQ(new_bucket.size(), 1);
	EXPECT_EQ(new_bucket[0].first, (LocationHash<precision, float, 4>::CoordinateArray{10.0f, 20.0f, 30.0f, 40.0f}));

	// Move a non-existing coordinate
	EXPECT_FALSE(locationHash.move({100.0f, 200.0f, 300.0f, 400.0f}, {150.0f, 250.0f, 350.0f, 450.0f}));
}

// Test adding, querying, and removing items in LocationHash with 2D coordinates
TEST(LocationHashTest, AddQueryRemove2DCoordinates)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 2D coordinates with no associated object
	LocationHash<precision, float, 2> locationHash;

	// Add some 2D coordinates
	locationHash.add({1.0f, 2.0f});
	locationHash.add({16.0f, 32.0f});

	// Query and ensure coordinates are present
	const auto & bucket1 = locationHash.query({1.0f, 2.0f});
	ASSERT_EQ(bucket1.size(), 1);
	EXPECT_EQ(bucket1[0].first, (LocationHash<precision, float, 2>::CoordinateArray{1.0f, 2.0f}));

	const auto & bucket2 = locationHash.query({16.0f, 32.0f});
	ASSERT_EQ(bucket2.size(), 1);
	EXPECT_EQ(bucket2[0].first, (LocationHash<precision, float, 2>::CoordinateArray{16.0f, 32.0f}));

	// Remove and ensure coordinates are removed
	EXPECT_TRUE(locationHash.remove({1.0f, 2.0f}));
	EXPECT_TRUE(locationHash.query({1.0f, 2.0f}).empty());

	EXPECT_TRUE(locationHash.remove({16.0f, 32.0f}));
	EXPECT_TRUE(locationHash.query({16.0f, 32.0f}).empty());
}

// Test adding, querying, and removing items in LocationHash with 3D coordinates and associated object
TEST(LocationHashTest, AddQueryRemove3DCoordinatesWithObject)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 3D coordinates with associated TestObject
	LocationHash<precision, double, 3, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, {1.0, 2.0, 3.0});
	locationHash.add(&obj2, {16.0, 32.0, 48.0});

	// Query and ensure coordinates and objects are present
	const auto & bucket1 = locationHash.query({1.0, 2.0, 3.0});
	ASSERT_EQ(bucket1.size(), 1);
	EXPECT_EQ(bucket1[0].second, &obj1);

	const auto & bucket2 = locationHash.query({16.0, 32.0, 48.0});
	ASSERT_EQ(bucket2.size(), 1);
	EXPECT_EQ(bucket2[0].second, &obj2);

	// Remove and ensure coordinates and objects are removed
	EXPECT_TRUE(locationHash.remove(&obj1, {1.0, 2.0, 3.0}));
	EXPECT_TRUE(locationHash.query({1.0, 2.0, 3.0}).empty());

	EXPECT_TRUE(locationHash.remove(&obj2, {16.0, 32.0, 48.0}));
	EXPECT_TRUE(locationHash.query({16.0, 32.0, 48.0}).empty());
}

// Test adding, querying, and removing items in LocationHash with 4D coordinates using epsilon for floating-point types
TEST(LocationHashTest, AddQueryRemove4DCoordinatesWithEpsilon)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 4D coordinates with no associated object
	LocationHash<precision, float, 4> locationHash;

	// Add some 4D coordinates
	locationHash.add({1.0f, 2.0f, 3.0f, 4.0f});
	locationHash.add({16.0f, 32.0f, 48.0f, 64.0f});

	// Query and ensure coordinates are present
	const auto & bucket1 = locationHash.query({1.0f, 2.0f, 3.0f, 4.0f});
	ASSERT_EQ(bucket1.size(), 1);
	EXPECT_EQ(bucket1[0].first, (LocationHash<precision, float, 4>::CoordinateArray{1.0f, 2.0f, 3.0f, 4.0f}));

	const auto & bucket2 = locationHash.query({16.0f, 32.0f, 48.0f, 64.0f});
	ASSERT_EQ(bucket2.size(), 1);
	EXPECT_EQ(bucket2[0].first, (LocationHash<precision, float, 4>::CoordinateArray{16.0f, 32.0f, 48.0f, 64.0f}));

	// Remove and ensure coordinates are removed
	EXPECT_TRUE(locationHash.remove({1.0f, 2.0f, 3.0f, 4.0f}));
	EXPECT_TRUE(locationHash.query({1.0f, 2.0f, 3.0f, 4.0f}).empty());

	EXPECT_TRUE(locationHash.remove({16.0f, 32.0f, 48.0f, 64.0f}));
	EXPECT_TRUE(locationHash.query({16.0f, 32.0f, 48.0f, 64.0f}).empty());
}

// Test adding an object with a radius so that it is placed in multiple buckets
TEST(LocationHashTest, AddObjectWithRadius)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 2D coordinates with associated TestObject
	LocationHash<precision, float, 2, TestObject> locationHash;

	// Create a test object with a radius
	TestObject obj1{1, "Object1"};

	// The location should be near a corner of the bucket so that inserting it with a radius
	// will place it in multiple buckets
	const auto keys = locationHash.add(&obj1, {15.0f, 15.0f}, 4.0f);

	// Query and ensure coordinates are present in multiple buckets
	ASSERT_EQ(keys.size(), 4);
	const auto & data = locationHash.get_data();
	for (const auto & key : keys) {
		const auto it = data.find(key);
		ASSERT_NE(it, data.end());
		ASSERT_EQ(it->second.size(), 1);
		EXPECT_EQ(it->second[0].second, &obj1);
	}
	ASSERT_EQ(keys[0].quantized_[0], 0);
	ASSERT_EQ(keys[0].quantized_[1], 0);
	ASSERT_EQ(keys[1].quantized_[0], 16);
	ASSERT_EQ(keys[1].quantized_[1], 0);
	ASSERT_EQ(keys[2].quantized_[0], 0);
	ASSERT_EQ(keys[2].quantized_[1], 16);
	ASSERT_EQ(keys[3].quantized_[0], 16);
	ASSERT_EQ(keys[3].quantized_[1], 16);

	// move with radius
	const auto movedKeys = locationHash.move(&obj1, 4.0f, {15.0f, 15.0f}, {21.0f, 21.0f});
	ASSERT_EQ(movedKeys.size(), 1);

	// cover the "zero move" case
	const auto zeroMovedKeys = locationHash.move(&obj1, 4.0f, {21.0f, 21.0f}, {21.0f, 21.0f});
	ASSERT_EQ(zeroMovedKeys.size(), 1); // fast return existing keys

	// cover the "small move" case
	const auto smallMovedKeys = locationHash.move(&obj1, 4.0f, {21.0f, 21.0f}, {21.1f, 21.1f});
	ASSERT_EQ(smallMovedKeys.size(), 1); // fast return existing keys
}

TEST(LocationHashTest, AddObjectWithRadius32)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 2D coordinates with associated TestObject
	LocationHash<precision, float, 2, TestObject, int32_t> locationHash;

	// Create a test object with a radius
	TestObject obj1{1, "Object1"};

	// The location should be near a corner of the bucket so that inserting it with a radius
	// will place it in multiple buckets
	const auto keys = locationHash.add(&obj1, {15.0f, 15.0f}, 4.0f);

	// Query and ensure coordinates are present in multiple buckets
	ASSERT_EQ(keys.size(), 4);
	const auto & data = locationHash.get_data();
	for (const auto & key : keys) {
		const auto it = data.find(key);
		ASSERT_NE(it, data.end());
		ASSERT_EQ(it->second.size(), 1);
		EXPECT_EQ(it->second[0].second, &obj1);
	}
	ASSERT_EQ(keys[0].quantized_[0], 0);
	ASSERT_EQ(keys[0].quantized_[1], 0);
	ASSERT_EQ(keys[1].quantized_[0], 16);
	ASSERT_EQ(keys[1].quantized_[1], 0);
	ASSERT_EQ(keys[2].quantized_[0], 0);
	ASSERT_EQ(keys[2].quantized_[1], 16);
	ASSERT_EQ(keys[3].quantized_[0], 16);
	ASSERT_EQ(keys[3].quantized_[1], 16);

	// move with radius
	const auto movedKeys = locationHash.move(&obj1, 4.0f, {15.0f, 15.0f}, {21.0f, 21.0f});
	ASSERT_EQ(movedKeys.size(), 1);

	// cover the "zero move" case
	const auto zeroMovedKeys = locationHash.move(&obj1, 4.0f, {21.0f, 21.0f}, {21.0f, 21.0f});
	ASSERT_EQ(zeroMovedKeys.size(), 1); // fast return existing keys
}

// ----------------------------------------------------------------------------
