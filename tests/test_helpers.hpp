#ifndef _INCLUDED_test_helpers_hpp
#define _INCLUDED_test_helpers_hpp
#include "gtest/gtest.h"
#include <vector>

/**
 * @brief
 * Order of complexity (Big O notation), from fastest to slowest.
 */
enum class Complexity {
  ERROR,       // Calculation failed or complexity cannot be determined
  O1,          // O(1) - Constant time
  OLogN,       // O(log n) - Logarithmic time
  ON,          // O(n) - Linear time
  ONLogN,      // O(n log n) - Linearithmic time
  ON2,         // O(n^2) - Quadratic time
  ON3,         // O(n^3) - Cubic time
  O2N,         // O(2^n) - Exponential time
  ONFactorial, // O(n!) - Factorial time
  OUnknown     // Complexity is not classified in the above categories
};

/**
 * @brief Helper function to convert Complexity enum to string.
 *
 * @param complexity
 * @return std::string
 */
std::string to_string(Complexity complexity);

/**
 * @brief
 * Measure the execution time of a lambda function for a given input size.
 * The lambda function is executed multiple times and the average time is
 * returned.
 *
 * @param setup
 * Executed before each lambda function execution to prepare the input for the
 * lambda function.
 *
 * @param lambda
 * The lambda function to measure. This function should take a single size_t for
 * input size.
 *
 * @param input_size
 * Parameter passed to setup and lambda functions.
 *
 * @param repetitions
 * The number of times to repeat the lambda function for each input size.
 * The average is calculated from this.
 *
 * @return double
 * The average time taken to execute the lambda function.
 */
double measure_execution_time(const std::function<void(size_t)> &setup,
                              const std::function<void(size_t)> &lambda,
                              size_t input_size, size_t repetitions);

Complexity determine_complexity(const std::vector<size_t> &input_sizes,
                                const std::vector<double> &times);
/**
 * @brief This function measures the execution time of a lambda function for a
 * given input size. The lambda function is executed multiple times and the
 * average time is returned.
 *
 * The setup function is called before each lambda function execution to prepare
 * the input for the lambda function.
 *
 * Time sampling is quite fuzzy and prone to a number of external factors on
 * where it is run, such as the scheduler, other loads on the machine, the
 * status of various cache levels. So, the test is run 3 times and the lowest
 * time is returned.
 *
 * Tests should compare the expected value is less than or equal to the next
 * greatest complexity, effectively setting some limits on the expected time and
 * failing the test if the underlying code is modified in a way that crosses
 * that boundary (say, from O(n) to O(n^2)).
 *
 * @param setup
 * The function to run before the lambda function. This function should prepare
 * the input for the lambda function.
 *
 * @param lambda
 * The lambda function to measure. This function should take a single size_t for
 * input size.
 *
 * @param input_sizes
 * A vector of input sizes to test.
 *
 * @param repetitions
 * The number of times to repeat the lambda function for each input size.
 *
 * @return Complexity, ERROR or Unknown if the complexity cannot be determined.
 */
Complexity
measure_time_complexity(const std::function<void(size_t input_size)> &setup,
                        const std::function<void(size_t input_size)> &lambda,
                        const std::vector<size_t> &input_sizes,
                        size_t repetitions = 10);

double linear_regression(const std::vector<double> &x,
                         const std::vector<double> &y);

#endif // _INCLUDED_test_helpers_hpp
