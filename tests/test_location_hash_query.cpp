#include "lochash/location_hash_query.hpp"
#include "lochash/lochash.hpp"
#include "gtest/gtest.h"

using namespace lochash;

// Test querying items in LocationHash with 2D coordinates
TEST(LocationHelpersTest, QueryBoundingBox2D)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 2D coordinates with associated TestObject
	LocationHash<precision, float, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};
	TestObject obj3{3, "Object3"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, 1.0f, 2.0f);
	locationHash.add(&obj2, 16.0f, 32.0f);
	locationHash.add(&obj3, 45.0f, 35.0f);

	// Query objects within a bounding box
	auto result = query_bounding_box(locationHash, std::make_tuple(0.0f, 0.0f), std::make_tuple(30.0f, 40.0f));

	// Check the result
	ASSERT_EQ(result.size(), 2);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj2) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj3) != result.end());
}

// Test querying items in LocationHash with 3D coordinates
TEST(LocationHelpersTest, QueryBoundingBox3D)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 3D coordinates with associated TestObject
	LocationHash<precision, double, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};
	TestObject obj3{3, "Object3"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, 1.0, 2.0, 3.0);
	locationHash.add(&obj2, 16.0, 32.0, 48.0);
	locationHash.add(&obj3, 25.0, 35.0, 55.0);

	// Query objects within a bounding box
	auto result = query_bounding_box(locationHash, std::make_tuple(0.0, 0.0, 0.0), std::make_tuple(30.0, 40.0, 50.0));

	// Check the result
	ASSERT_EQ(result.size(), 2);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj2) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj3) != result.end());
}

// Test querying items in LocationHash with 4D coordinates
TEST(LocationHelpersTest, QueryBoundingBox4D)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 4D coordinates with associated TestObject
	LocationHash<precision, float, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};
	TestObject obj3{3, "Object3"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, 1.0f, 2.0f, 3.0f, 4.0f);
	locationHash.add(&obj2, 16.0f, 32.0f, 48.0f, 64.0f);
	locationHash.add(&obj3, 25.0f, 35.0f, 45.0f, 155.0f);

	// Query objects within a bounding box
	auto result = query_bounding_box(locationHash, std::make_tuple(0.0f, 0.0f, 0.0f, 0.0f),
	                                 std::make_tuple(30.0f, 40.0f, 50.0f, 60.0f));

	// Check the result
	ASSERT_EQ(result.size(), 1);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj2) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj3) != result.end());
}
