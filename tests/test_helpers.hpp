#include "gtest/gtest.h"
#include <initializer_list>

enum class ComplexityThreshold { O1, ON, OLOGN, ON2, ON3 };

/**
 * Test the complexity of an algorithm by comparing the timings of different data points. Recommend using std::chrono or
 * other high-precision timers.
 *
 * @param counts The number of elements for each data point.
 * @param timings The timings for each data point.
 * @param threshold The complexity threshold to test against.
 */
void test_complexity(std::initializer_list<std::size_t> counts, std::initializer_list<long long> timings,
                     ComplexityThreshold threshold);
