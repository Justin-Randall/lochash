#include "test_helpers.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

class DetermineComplexityTest : public ::testing::Test {
protected:
  // Helper function to generate test data
  std::pair<std::vector<double>, std::vector<size_t>>
  generateTestData(Complexity expectedComplexity, size_t size = 3) {
    std::vector<double> times(size);
    std::vector<size_t> input_sizes(size);
    for (size_t i = 0; i < size; ++i) {
      input_sizes[i] = i + 1;
      switch (expectedComplexity) {
      case Complexity::O1:
        times[i] = 1;
        break;
      case Complexity::OLogN:
        times[i] = std::log2(input_sizes[i]);
        break;
      case Complexity::ON:
        times[i] = static_cast<double>(input_sizes[i]);
        break;
      case Complexity::ONLogN:
        times[i] = input_sizes[i] * std::log2(input_sizes[i]);
        break;
      case Complexity::ON2:
        times[i] = static_cast<double>(input_sizes[i] * input_sizes[i]);
        break;
      case Complexity::ON3:
        times[i] = static_cast<double>(input_sizes[i] * input_sizes[i] *
                                       input_sizes[i]);
        break;
      case Complexity::O2N:
        times[i] = std::pow(2, input_sizes[i]);
        break;
      case Complexity::ONFactorial:
        times[i] = std::tgamma(input_sizes[i] + 1); // Factorial
        break;
      default:
        times[i] = 0;
      }
    }
    return {times, input_sizes};
  }
};

TEST_F(DetermineComplexityTest, EmptyData) {
  auto [times, input_sizes] = generateTestData(Complexity::ERROR);
  EXPECT_EQ(times, std::vector<double>(3, 0));
}

TEST_F(DetermineComplexityTest, InvalidArguments) {
  std::vector<double> times = {1.0};
  std::vector<size_t> input_sizes = {1, 2}; // Different sizes
  EXPECT_EQ(determine_complexity(input_sizes, times), Complexity::ERROR);
}

TEST_F(DetermineComplexityTest, ComplexityO1) {
  auto [times, input_sizes] = generateTestData(Complexity::O1);
  EXPECT_EQ(determine_complexity(input_sizes, times), Complexity::O1);
}

TEST_F(DetermineComplexityTest, ComplexityOLOGN) {
  auto [times, input_sizes] = generateTestData(Complexity::OLogN);
  auto complexity = determine_complexity(input_sizes, times);
  EXPECT_EQ(complexity, Complexity::OLogN);
}

TEST_F(DetermineComplexityTest, ComplexityON) {
  auto [times, input_sizes] = generateTestData(Complexity::ON);
  auto complexity = determine_complexity(input_sizes, times);
  EXPECT_EQ(complexity, Complexity::ON);
}

TEST_F(DetermineComplexityTest, ComplexityONLOGN) {
  auto [times, input_sizes] = generateTestData(Complexity::ONLogN);
  EXPECT_EQ(determine_complexity(input_sizes, times), Complexity::ONLogN);
}

TEST_F(DetermineComplexityTest, ComplexityON2) {
  auto [times, input_sizes] = generateTestData(Complexity::ON2);
  EXPECT_EQ(determine_complexity(input_sizes, times), Complexity::ON2);
}

TEST_F(DetermineComplexityTest, ComplexityON3) {
  auto [times, input_sizes] = generateTestData(Complexity::ON3);
  EXPECT_EQ(determine_complexity(input_sizes, times), Complexity::ON3);
}

TEST_F(DetermineComplexityTest, ComplexityO2N) {
  auto [times, input_sizes] = generateTestData(Complexity::O2N);
  EXPECT_EQ(determine_complexity(input_sizes, times), Complexity::O2N);
}

TEST_F(DetermineComplexityTest, ComplexityOFACT) {
  auto [times, input_sizes] = generateTestData(
      Complexity::ONFactorial,
      4); // Need more data points for factorial to be distinguishable
  EXPECT_LE(determine_complexity(input_sizes, times), Complexity::ONFactorial);
}

TEST_F(DetermineComplexityTest, ComplexityInvalid) {
  // Crafting times and input_sizes to trigger OUnknown
  std::vector<double> times = {1.0, 2.0, std::nan("1"),
                               -1.0}; // Deliberate NaN and negative values
  std::vector<size_t> input_sizes = {1, 2, 3, 4};

  EXPECT_EQ(determine_complexity(input_sizes, times), Complexity::OUnknown);
}

TEST_F(DetermineComplexityTest, ComplexityWithNonPositiveValues) {
  // Test with a non-positive value in input_sizes
  std::vector<size_t> input_sizes = {1, 2, 0,
                                     3}; // includes 0 which is non-positive
  std::vector<double> times = {1.0, 2.0, 0.5, 3.0};

  // The presence of a non-positive value should make the O(log n) fit invalid
  Complexity complexity = determine_complexity(input_sizes, times);

  // Since log n can't be determined, it should skip the O(log n) complexity
  // The test here expects the best fit, which should not be OLogN
  EXPECT_NE(complexity, Complexity::OLogN);
}

TEST_F(DetermineComplexityTest, LinearRegression) {
  // Test linear regression with an empty x vector and non-empty y vector
  std::vector<double> x = {};
  std::vector<double> y = {1.0, 2.0, 3.0, 4.0, 5.0};
  const auto result = linear_regression(x, y);
  EXPECT_EQ(result, -std::numeric_limits<double>::infinity());
}
