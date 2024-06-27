#include "lochash/lochash.hpp"
#include "gtest/gtest.h"

using namespace lochash;

struct TestObject {
	int         id;
	std::string name;

	int map_x;
	int map_y;
	int map_z;

	float x;
	float y;
	float z;
};

// Test recursive use of the LocationHash
TEST(LocationHashRecursion, QueryBoundingBox3D)
{
	// Creating recursive LocationHashes for a 3d environment.
	// Player will be in some floating point coordinate system
	// on an infinite map, with local queries within a map.
	constexpr size_t player_precision      = 16;
	constexpr size_t map_of_maps_precision = 16;

	TestObject player1(1, "Player1", 0, 0, 0, 0.0f, 0.0f, 0.0f);
	TestObject player2(2, "Player2", 0, 0, 0, 10.0, 10.0, 10.0);
	TestObject player3(3, "Player3", 1, 0, 0, 20.0, 20.0, 20.0);

	LocationHash<player_precision, float, 3, TestObject>                                              map1Hash;
	LocationHash<player_precision, float, 3, TestObject>                                              map2Hash;
	LocationHash<map_of_maps_precision, int, 3, LocationHash<player_precision, float, 3, TestObject>> mapOfMapsHash;

	map1Hash.add(&player1, {player1.x, player1.y, player1.z});
	map1Hash.add(&player2, {player2.x, player2.y, player2.z});
	map2Hash.add(&player3, {player3.x, player3.y, player3.z});

	mapOfMapsHash.add(&map1Hash, {player1.map_x, player1.map_y, player1.map_z});
	mapOfMapsHash.add(&map2Hash, {player3.map_x, player3.map_y, player3.map_z});

	// ensure the mapOfMapsHash contain the map hashes
	const auto & mapOfMapBuckets1 = mapOfMapsHash.query({player1.map_x, player1.map_y, player1.map_z});
	// both maps should be in the same bucket
	ASSERT_EQ(mapOfMapBuckets1.size(), 2);

	const auto & r1 = mapOfMapBuckets1[0].second;
	const auto & r2 = mapOfMapBuckets1[1].second;

	EXPECT_EQ(r1, &map1Hash);
	EXPECT_EQ(r2, &map2Hash);

	// ensure the objects exist in the r1 and r2 buckets respectively, and in ones they should not exist in.
	const auto & map1Bucket = r1->query({player1.x, player1.y, player1.z});
	// player1 and player2 should be in map1Bucket
	ASSERT_EQ(map1Bucket.size(), 2);
	const auto & pr1 = map1Bucket[0].second;
	const auto & pr2 = map1Bucket[1].second;
	EXPECT_EQ(pr1, &player1);
	EXPECT_EQ(pr2, &player2);

	// repeat for player3 in r2
	const auto & map2Bucket = r2->query({player3.x, player3.y, player3.z});
	// player3 should be in map2Bucket
	ASSERT_EQ(map2Bucket.size(), 1);
	const auto & pr3 = map2Bucket[0].second;
	EXPECT_EQ(pr3, &player3);
}