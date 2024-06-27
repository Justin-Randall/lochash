#include "lochash/location_hash_query_bounding_box.hpp"
#include "lochash/lochash.hpp"
#include "test_helpers.hpp"
#include "gtest/gtest.h"

using namespace lochash;

struct TestObject {
	int         id;
	std::string name;
};

// Test querying items in LocationHash with 2D coordinates
TEST(LocationHelpersTest, QueryBoundingBox2D)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 2D coordinates with associated TestObject
	LocationHash<precision, float, 2, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};
	TestObject obj3{3, "Object3"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, {1.0f, 2.0f});
	locationHash.add(&obj2, {16.0f, 32.0f});
	locationHash.add(&obj3, {45.0f, 35.0f});

	// Query objects within a bounding box
	auto result = query_bounding_box(locationHash, {0.0f, 0.0f}, {30.0f, 40.0f});

	// Check the result
	ASSERT_EQ(result.size(), 2);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj2) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj3) != result.end());
}

// Test querying items in LocationHash with 3D coordinates
TEST(LocationHelpersTest, QueryBoundingBox3D)
{
	constexpr size_t precision = 16;

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

	// Query objects within a bounding box
	auto result = query_bounding_box(locationHash, {0.0, 0.0, 0.0}, {30.0, 40.0, 50.0});

	// Check the result
	ASSERT_EQ(result.size(), 2);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj2) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj3) != result.end());
}

// Test querying items in LocationHash with 4D coordinates
TEST(LocationHelpersTest, QueryBoundingBox4D)
{
	constexpr size_t precision = 16;

	// Create a LocationHash for 4D coordinates with associated TestObject
	LocationHash<precision, float, 4, TestObject> locationHash;

	// Create some test objects
	TestObject obj1{1, "Object1"};
	TestObject obj2{2, "Object2"};
	TestObject obj3{3, "Object3"};

	// Add coordinates with associated objects
	locationHash.add(&obj1, {1.0f, 2.0f, 3.0f, 4.0f});
	locationHash.add(&obj2, {16.0f, 32.0f, 48.0f, 64.0f});
	locationHash.add(&obj3, {25.0f, 35.0f, 45.0f, 155.0f});

	// Query objects within a bounding box
	const auto result = query_bounding_box(locationHash, {0.0f, 0.0f, 0.0f, 0.0f}, {30.0f, 40.0f, 50.0f, 60.0f});

	// Check the result
	ASSERT_EQ(result.size(), 1);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj2) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj3) != result.end());
}

TEST(LocationHelpersTest, QueryBoundingBoxComplexity)
{
	constexpr size_t                              precision = 16;
	LocationHash<precision, float, 2, TestObject> locationHash;

	auto setup = [&](size_t count) {
		locationHash.clear();
		std::vector<TestObject> test_objects;
		test_objects.reserve(count);
		for (int i = 0; i < count; ++i) {
			test_objects.push_back({i, "Object" + std::to_string(i)});
			const float x = -1000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2000.0f));
			const float y = -1000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2000.0f));
			locationHash.add(&test_objects[i], {x, y});
		}
	};
	ComplexityThreshold complexity = measure_time_complexity(
	    setup,
	    [&](size_t) {
		    query_bounding_box(locationHash, {-50.0f, -50.0f}, {50.0f, 50.0f});
	    },
	    {10, 100, 1000}, 5);

	const ComplexityThreshold expectedComplexity = ComplexityThreshold::O1;
	EXPECT_LE(complexity, expectedComplexity)
	    << "QueryBoundingBoxComplexityLabmdas test failed. Expected complexity threshold not met. Reported complexity: "
	    << to_string(complexity) << " Expected complexity: " << to_string(expectedComplexity);
}
