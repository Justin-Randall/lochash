#include "test_helpers.hpp"

ComplexityThreshold test_complexity(const std::vector<std::size_t> & counts, const std::vector<long long> & timings)
{
	auto it_counts  = counts.begin();
	auto it_timings = timings.begin();

	// Ensure that we have at least two data points
	if (counts.size() < 2 || timings.size() < 2) {
		throw std::invalid_argument("At least two data points are required");
	}

	ComplexityThreshold determined_threshold = ComplexityThreshold::O1;

	while (std::next(it_counts) != counts.end() && std::next(it_timings) != timings.end()) {
		double n_ratio    = static_cast<double>(*std::next(it_counts)) / *it_counts;
		double time_ratio = static_cast<double>(*std::next(it_timings)) / *it_timings;

		double expected_ologn_ratio  = std::log2(n_ratio);
		double expected_on_ratio     = n_ratio;
		double expected_onlogn_ratio = n_ratio * std::log2(n_ratio);
		double expected_on2_ratio    = n_ratio * n_ratio;
		double expected_on3_ratio    = n_ratio * n_ratio * n_ratio;
		double expected_o2n_ratio    = std::pow(2, n_ratio);
		double expected_ofact_ratio  = std::tgamma(n_ratio + 1);

		if (time_ratio < expected_ologn_ratio) {
			determined_threshold = ComplexityThreshold::O1;
		} else if (time_ratio < expected_on_ratio) {
			determined_threshold = ComplexityThreshold::OLOGN;
		} else if (time_ratio < expected_onlogn_ratio) {
			determined_threshold = ComplexityThreshold::ON;
		} else if (time_ratio < expected_on2_ratio) {
			determined_threshold = ComplexityThreshold::ONLOGN;
		} else if (time_ratio < expected_on3_ratio) {
			determined_threshold = ComplexityThreshold::ON2;
		} else if (time_ratio < expected_o2n_ratio) {
			determined_threshold = ComplexityThreshold::ON3;
		} else if (time_ratio < expected_ofact_ratio) {
			determined_threshold = ComplexityThreshold::O2N;
		} else {
			determined_threshold = ComplexityThreshold::OFACT;
		}

		++it_counts;
		++it_timings;
	}

	return determined_threshold;
}
