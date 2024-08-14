#include "lochash/location_hash_quantized_coordinate.hpp"
#include "lochash/lochash.hpp"
#include "gtest/gtest.h"

using namespace lochash;

TEST(LocationQuantizedCoordinateTest, CoordinateTypes)
{
	constexpr size_t precision  = 128;
	using CoordinateType        = double;
	constexpr size_t dimensions = 3;

	std::array<double, 3>                              min_coords = {0.0, 0.0, 0.0};
	QuantizedCoordinate<precision, CoordinateType, dimensions> qc1(min_coords);
	QuantizedCoordinate<precision, CoordinateType, dimensions> qc2(min_coords);

	EXPECT_EQ(qc1, qc2);
}
