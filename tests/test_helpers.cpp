#include "test_helpers.hpp"
// #include <chrono>

void test_complexity(std::initializer_list<std::size_t> counts, std::initializer_list<long long> timings,
                     ComplexityThreshold threshold)
{
	SCOPED_TRACE("Testing complexity");
	auto it_counts  = counts.begin();
	auto it_timings = timings.begin();

	// Ensure that we have at least two data points
	EXPECT_TRUE(counts.size() >= 2);
	EXPECT_TRUE(timings.size() >= 2);

	while (std::next(it_counts) != counts.end() && std::next(it_timings) != timings.end()) {
		double n_ratio    = static_cast<double>(*std::next(it_counts)) / *it_counts;
		double time_ratio = static_cast<double>(*std::next(it_timings)) / *it_timings;

		double expected_ologn_ratio = std::log2(n_ratio);
		double expected_on_ratio    = n_ratio;
		double expected_on2_ratio   = n_ratio * n_ratio;
		double expected_on3_ratio   = n_ratio * n_ratio * n_ratio;

		switch (threshold) {
		case ComplexityThreshold::O1: {
			SCOPED_TRACE("The measured time ratio at count should is expected to be between O(1) and O(log(n))");
			EXPECT_TRUE(time_ratio < expected_ologn_ratio);
		} break;
		case ComplexityThreshold::OLOGN: {
			SCOPED_TRACE("The measured time ratio at count should is expected to be between O(log(n)) and O(n)");
			EXPECT_TRUE(time_ratio < expected_on_ratio);
		} break;
		case ComplexityThreshold::ON: {
			SCOPED_TRACE("The measured time ratio at count should is expected to be between O(n) and O(n^2)");
			EXPECT_TRUE(time_ratio < expected_on2_ratio);
		} break;
		case ComplexityThreshold::ON2: {
			SCOPED_TRACE("The measured time ratio at count should is expected to be between O(n^2) and O(n^3)");
			EXPECT_TRUE(time_ratio < expected_on3_ratio);
		} break;
		case ComplexityThreshold::ON3:
			// No higher threshold to compare to, assuming acceptable
			EXPECT_TRUE(true);
			break;
		}

		++it_counts;
		++it_timings;
	}
}
