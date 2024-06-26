#include "lochash/location_hash_query_distance_squared.hpp"
#include "gtest/gtest.h"

using namespace lochash;

// Define a custom type for testing with associated objects
struct TestObject {
	int         id;
	std::string name;
};

// Test querying items within a certain distance from a point
TEST(LocationHashQueryTest, QueryWithinDistance2D)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 2D coordinates with associated TestObject
	LocationHash<precision, float, 2, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};
	TestObject obj3{3, "Object3"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, {4.0f, 3.0f});
	locationHash.add(&obj2, {16.0f, 32.0f});
	locationHash.add(&obj3, {45.0f, 35.0f});

	// Query objects within a distance of 5 units from (0,0)
	auto result = query_within_distance(locationHash, {5.5f, 5.5f}, 5.0f);

	// Check the result
	ASSERT_EQ(result.size(), 1);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
}

// Test querying items within a certain distance from a point in 3D
TEST(LocationHashQueryTest, QueryWithinDistance3D)
{
	constexpr std::size_t precision = 16;

	// Create a LocationHash for 3D coordinates with associated TestObject
	LocationHash<precision, double, 3, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};
	TestObject obj3{3, "Object3"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, {1.0, 2.0, 3.0});
	locationHash.add(&obj2, {16.0, 32.0, 48.0});
	locationHash.add(&obj3, {25.0, 35.0, 55.0});

	// Query objects within a distance of 10 units from (0,0,0)
	const auto result = query_within_distance(locationHash, {0.0, 0.0, 0.0}, 10.0);

	// Check the result
	ASSERT_EQ(result.size(), 1);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
}
