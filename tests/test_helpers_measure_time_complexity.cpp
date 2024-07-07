#include "test_helpers.hpp"
#include <gtest/gtest.h>

// Test case for constant time complexity O(1)
TEST(MeasureTimeComplexityTest, ConstantTimeComplexity) {
  auto setup = [](size_t input_size) {};
  auto lambda = [](size_t input_size) {
    std::this_thread::yield();
    return;
  };

  std::vector<size_t> input_sizes = {100, 200, 300, 400, 500};
  Complexity complexity = measure_time_complexity(setup, lambda, input_sizes);

  EXPECT_LE(complexity, Complexity::OLogN);
}

// Test case for logarithmic time complexity O(log n)
TEST(MeasureTimeComplexityTest, LogarithmicTimeComplexity) {
  auto setup = [](size_t input_size) {};
  auto lambda = [](size_t input_size) {
    for (size_t i = 1; i < input_size; i *= 2) {
      std::this_thread::yield();
    }
  };

  std::vector<size_t> input_sizes = {100,  200,  400,   800,   1600,
                                     3200, 6400, 12800, 25600, 51200};
  Complexity complexity = measure_time_complexity(setup, lambda, input_sizes);

  EXPECT_LE(complexity, Complexity::ON);
}

// Test case for linear time complexity O(n)
TEST(MeasureTimeComplexityTest, LinearTimeComplexity) {
  auto setup = [](size_t input_size) {};
  auto lambda = [](size_t input_size) {
    for (size_t i = 0; i < input_size; ++i) {
      std::this_thread::yield();
    }
  };

  std::vector<size_t> input_sizes = {100, 200, 300, 400, 500,
                                     600, 700, 800, 900, 1000};
  Complexity complexity = measure_time_complexity(setup, lambda, input_sizes);

  EXPECT_LE(complexity, Complexity::ONLogN);
}

// Test case for quadratic time complexity O(n^2)
TEST(MeasureTimeComplexityTest, QuadraticTimeComplexity) {
  auto setup = [](size_t input_size) {};
  auto lambda = [](size_t input_size) {
    for (size_t i = 0; i < input_size; ++i) {
      for (size_t j = 0; j < input_size; ++j) {
      }
    }
  };

  std::vector<size_t> input_sizes = {10, 20, 30, 40, 50};
  Complexity complexity = measure_time_complexity(setup, lambda, input_sizes);

  EXPECT_LE(complexity, Complexity::ON3);
}

// Test case for cubic time complexity O(n^3)
TEST(MeasureTimeComplexityTest, CubicTimeComplexity) {
  auto setup = [](size_t input_size) {};
  auto lambda = [](size_t input_size) {
    for (size_t i = 0; i < input_size; ++i) {
      for (size_t j = 0; j < input_size; ++j) {
        for (size_t k = 0; k < input_size; ++k) {
        }
      }
    }
  };

  std::vector<size_t> input_sizes = {10, 20, 30, 40, 50};
  Complexity complexity = measure_time_complexity(setup, lambda, input_sizes);

  EXPECT_LE(complexity, Complexity::O2N);
}
