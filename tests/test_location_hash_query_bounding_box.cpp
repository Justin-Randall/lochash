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
	constexpr std::size_t precision = 16;

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
	constexpr std::size_t precision = 16;

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
	auto result = query_bounding_box(locationHash, {0.0f, 0.0f, 0.0f, 0.0f}, {30.0f, 40.0f, 50.0f, 60.0f});

	// Check the result
	ASSERT_EQ(result.size(), 1);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj2) != result.end());
	EXPECT_FALSE(std::find(result.begin(), result.end(), &obj3) != result.end());
}

TEST(LocationHelpersTest, QueryBoundingBoxComplexity)
{
	std::vector<std::size_t> object_counts = {10, 100, 1000, 10000};
	std::vector<long long>   query_times;

	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	constexpr std::size_t   max_objects = 10000;
	std::vector<TestObject> test_objects;
	test_objects.reserve(max_objects);
	for (std::size_t i = 0; i < max_objects; ++i) {
		test_objects.push_back({static_cast<int>(i), "Object" + std::to_string(i)});
	}

	constexpr std::size_t precision = 16;
	for (auto count : object_counts) {
		LocationHash<precision, float, 2, TestObject> locationHash;

		for (int i = 0; i < count; ++i) {
			float x = -1000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2000.0f));
			float y = -1000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2000.0f));
			locationHash.add(&test_objects[i], {x, y});
		}

		auto start = std::chrono::high_resolution_clock::now();

		auto result = query_bounding_box(locationHash, {-50.0f, -50.0f}, {50.0f, 50.0f});

		auto end      = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		query_times.push_back(duration);
	}

	// Test the complexity against an expected threshold (e.g., O(log n))
	auto determined_complexity = test_complexity(object_counts, query_times);
	EXPECT_LE(determined_complexity, ComplexityThreshold::O1);
}
