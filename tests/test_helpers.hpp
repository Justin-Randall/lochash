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

/**
 * Test the complexity of an algorithm by comparing the timings of different data points. Recommend using std::chrono or
 * other high-precision timers.
 *
 * The threshold returned is the highest complexity threshold that the algorithm fits within. For example, if the
 * algorithm fits within O(n) and O(n log n), the threshold will be O(n).
 *
 * OFACT is the slowest complexity threshold and means the algorithm is at least as bad as traveling salesman brute
 * force.
 *
 * @param counts The number of elements for each data point.
 * @param timings The timings for each data point.
 * @return The determined complexity threshold.
 */
ComplexityThreshold test_complexity(const std::vector<std::size_t> & counts, const std::vector<long long> & timings);
