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

// ----------------------------------------------------------------------------
