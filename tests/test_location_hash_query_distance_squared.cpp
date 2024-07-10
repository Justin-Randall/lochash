#include "lochash/location_hash_query_distance_squared.hpp"
#include "test_helpers.hpp"
#include "gtest/gtest.h"

using namespace lochash;

// Define a custom type for testing with associated objects
struct TestObject {
	size_t      id;
	std::string name;
};

// Test querying items within a certain distance from a point
TEST(LocationHashQueryTest, QueryWithinDistance2D)
{
	constexpr size_t precision = 16;

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

	// Query objects within a distance of 10 units from (0,0,0)
	const auto result = query_within_distance(locationHash, {0.0, 0.0, 0.0}, 10.0);

	// Check the result
	ASSERT_EQ(result.size(), 1);
	EXPECT_TRUE(std::find(result.begin(), result.end(), &obj1) != result.end());
}

TEST(LocationHelpersTest, QueryDistanceComplexity)
{
	constexpr size_t                              precision = 16;
	LocationHash<precision, float, 2, TestObject> locationHash;

	auto setup = [&](size_t count) {
		locationHash.clear();
		std::vector<TestObject> test_objects;
		test_objects.reserve(count);
		for (size_t i = 0; i < count; ++i) {
			test_objects.push_back({i, "Object" + std::to_string(i)});
			const float x = -1000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2000));
			const float y = -1000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2000));
			locationHash.add(&test_objects[i], {x, y});
		}
	};
	Complexity complexity = measure_time_complexity(
	    setup,
	    [&](size_t) {
		    query_within_distance(locationHash, {0.0f, 0.0f}, 500.0f);
	    },
	    {10, 100, 1000}, 5);

	const Complexity expectedComplexity = Complexity::O1;
	EXPECT_LE(complexity, expectedComplexity)
	    << "QueryBoundingBoxComplexityLabmdas test failed. Expected complexity threshold not met. Reported complexity: "
	    << to_string(complexity) << " Expected complexity: " << to_string(expectedComplexity);
}

TEST(GenerateAllHashKeysWithinDistanceTest, ReturnsCorrectHashKeysFor2D)
{
	constexpr size_t     precision = 4;
	std::array<float, 2> center    = {0.0f, 0.0f};
	float                radius    = 5.0f;
	auto                 result    = generate_all_hash_keys_within_distance<precision, float, 2>(center, radius);
	EXPECT_EQ(result.size(), 9);
}

TEST(GenerateAllHashKeysWithinDistanceTest, PrecisionIsPowerOfTwo)
{
	// This test ensures that the static assertion for precision being a power of two is in effect.
	constexpr size_t   precision = 2;   // This should be a power of two for the test to compile
	std::array<int, 1> center    = {0}; // Single dimension array for simplicity
	int                radius    = 1;
	auto               result    = generate_all_hash_keys_within_distance<precision, int, 1>(center, radius);
	// No need for runtime assertion here, compilation success is the test
}

// This test is expected to fail compilation due to the static_assert if uncommented
// TEST(GenerateAllHashKeysWithinDistanceTest, FailsForNonArithmeticType) {
//     constexpr size_t precision = 4;
//     struct NonArithmetic {};
//     std::array<NonArithmetic, 1> center = {{}};
//     NonArithmetic radius = {};
//     auto result = generate_all_hash_keys_within_distance<precision, NonArithmetic, 1>(center, radius);
//     // This test is designed to fail at compile time due to static_assert
// }
