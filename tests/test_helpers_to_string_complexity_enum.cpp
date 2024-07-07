#include "test_helpers.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(ComplexityThresholdToStringTest, ConvertsEnumToString) {
  EXPECT_EQ(to_string(Complexity::ERROR), "ERROR");
  EXPECT_EQ(to_string(Complexity::O1), "O(1)");
  EXPECT_EQ(to_string(Complexity::OLogN), "O(log n)");
  EXPECT_EQ(to_string(Complexity::ON), "O(n)");
  EXPECT_EQ(to_string(Complexity::ONLogN), "O(n log n)");
  EXPECT_EQ(to_string(Complexity::ON2), "O(n^2)");
  EXPECT_EQ(to_string(Complexity::ON3), "O(n^3)");
  EXPECT_EQ(to_string(Complexity::O2N), "O(2^n)");
  EXPECT_EQ(to_string(Complexity::ONFactorial), "O(n!)");
  EXPECT_EQ(to_string(Complexity::OUnknown), "O(Unknown)");

  // Failure case
  EXPECT_EQ(to_string(static_cast<Complexity>(-1)), "Undefined");
}
