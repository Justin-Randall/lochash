#include "lochash/location_hash_query_distance_squared.hpp"
#include "test_helpers.hpp"
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

TEST(LocationHelpersTest, QueryDistanceComplexity)
{
	const std::vector<std::size_t> object_counts = {10, 100, 1000, 10000};
	std::vector<long long>         query_times;

	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	constexpr std::size_t   max_objects = 10000;
	std::vector<TestObject> test_objects;
	test_objects.reserve(max_objects);
	for (std::size_t i = 0; i < max_objects; ++i) {
		test_objects.push_back({static_cast<int>(i), "Object" + std::to_string(i)});
	}

	constexpr std::size_t precision = 16;
	for (const auto count : object_counts) {
		LocationHash<precision, float, 2, TestObject> locationHash;

		for (int i = 0; i < count; ++i) {
			const float x = -1000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2000.0f));
			const float y = -1000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2000.0f));
			locationHash.add(&test_objects[i], {x, y});
		}

		const auto start = std::chrono::high_resolution_clock::now();

		const auto result = query_within_distance(locationHash, {0.0f, 0.0f}, 500.0f);

		const auto end      = std::chrono::high_resolution_clock::now();
		const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		query_times.push_back(duration);
	}

	// Test the complexity against an expected threshold (e.g., O(log n))
	const auto determined_complexity = test_complexity(object_counts, query_times);
	EXPECT_LE(determined_complexity, ComplexityThreshold::O1);
}
