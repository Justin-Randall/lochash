#ifndef _INCLUDED_test_helpers_hpp
#define _INCLUDED_test_helpers_hpp
#include "gtest/gtest.h"
#include <vector>

/**
 * @brief
 * Order of complexity (Big O notation), from fastest to slowest.
 */
enum class ComplexityThreshold {
	O1,     // O(1). Time ratio should be less than O(log n).
	OLOGN,  // O(log n). Time ratio should be less than O(n).
	ON,     // O(n). Time ratio should be less than O(n log n).
	ONLOGN, // O(n log n). Time ratio should be less than O(n^2).
	ON2,    // O(n^2). Time ratio should be less than O(n^3).
	ON3,    // O(n^3). Time ratio should be less than O(2^n).
	O2N,    // O(2^n). Time ratio should be less than O(n!).
	OFACT   // O(n!). Time ratio should be greater than O(n!).
};

std::string to_string(ComplexityThreshold threshold);

/**
 * @brief Measure the asymptotic time complexity of a function.
 *
 * @param setup
 * A setup function that prepares the input for the lambda function. The setup function should take a single size_t for
 * input size.
 *
 * @param lambda
 * The lambda function to measure. The lambda function should take a single size_t for input size.
 *
 * @param input_sizes
 * A vector of input sizes to test.
 *
 * @param repetitions
 * The number of times to repeat the lambda function for each input size.
 *
 * @return ComplexityThreshold
 */
ComplexityThreshold measure_time_complexity(const std::function<void(size_t input_size)> & setup,
                                            const std::function<void(size_t input_size)> & lambda,
                                            const std::vector<size_t> & input_sizes, size_t repetitions = 10);

#endif // _INCLUDED_test_helpers_hpp
