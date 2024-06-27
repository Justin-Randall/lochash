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
 * If 4 runs of 20, 200, 2000, and 20000 elements take the same amount of time, the complexity is O(1). If the time
 * increases linearly with the number of elements, the complexity is O(n). If the time increases logarithmically with
 * the number of elements, the complexity is O(log n). If the time increases linearithmically with the number of
 * elements, the complexity is O(n log n). If the time increases quadratically with the number of elements, the
 * complexity is O(n^2). If the time increases cubically with the number of elements, the complexity is O(n^3). If the
 * time increases exponentially with the number of elements, the complexity is O(2^n). If the time increases factorially
 * with the number of elements, the complexity is O(n!).
 *
 * The threshold should be one order of complexity higher than expected, since perfect implementations and noise with
 * timing will almost always cause measurements to be slightly higher than expected in a perfect implementation. If the
 * expected complexity is O(1), the threshold should be O(log n) because it means that is provably not performing with
 * O(1) complexity.
 *
 * @param counts The number of elements for each data point.
 * @param timings The timings for each data point.
 * @return The determined complexity threshold.
 */
ComplexityThreshold test_complexity(const std::vector<std::size_t> & counts, const std::vector<long long> & timings);
