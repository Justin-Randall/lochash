#include "test_helpers.hpp"
#include "gtest/gtest.h"

std::string to_string(ComplexityThreshold threshold)
{
	// convert enum to string
	switch (threshold) {
	case ComplexityThreshold::O1:
		return "O(1)";
	case ComplexityThreshold::OLOGN:
		return "O(log n)";
	case ComplexityThreshold::ON:
		return "O(n)";
	case ComplexityThreshold::ONLOGN:
		return "O(n log n)";
	case ComplexityThreshold::ON2:
		return "O(n^2)";
	case ComplexityThreshold::ON3:
		return "O(n^3)";
	case ComplexityThreshold::O2N:
		return "O(2^n)";
	case ComplexityThreshold::OFACT:
		return "O(n!)";
	default:
		return "Unknown";
	}
}

double measure_execution_time(const std::function<void(size_t)> & setup, const std::function<void(size_t)> & lambda,
                              size_t input_size, size_t repetitions)
{
	using namespace std::chrono;
	double total_time = 0.0;

	for (int i = 0; i < repetitions; ++i) {
		setup(input_size);

		auto start = high_resolution_clock::now();
		lambda(input_size);
		auto             end     = high_resolution_clock::now();
		duration<double> elapsed = end - start;
		total_time += elapsed.count();
	}

	double average_time = total_time / repetitions;
	return average_time;
}

ComplexityThreshold determine_complexity(const std::vector<double> & times, const std::vector<size_t> & input_sizes)
{
	if (times.size() != input_sizes.size() || times.size() < 2) {
		throw std::invalid_argument("Input vectors must be of the same size and contain at least two elements.");
	}

	std::vector<double> ratios;

	for (size_t i = 1; i < times.size(); ++i) {
		ratios.push_back(times[i] / times[i - 1]);
	}

	for (size_t i = 1; i < input_sizes.size(); ++i) {
		double log_ratio =
		    std::log2(static_cast<double>(input_sizes[i])) / std::log2(static_cast<double>(input_sizes[i - 1]));

		if (ratios[i - 1] < log_ratio) {
			return ComplexityThreshold::O1;
		} else if (ratios[i - 1] < static_cast<double>(input_sizes[i]) / input_sizes[i - 1]) {
			return ComplexityThreshold::OLOGN;
		} else if (ratios[i - 1] < static_cast<double>(input_sizes[i] * std::log2(input_sizes[i])) /
		                               (input_sizes[i - 1] * std::log2(input_sizes[i - 1]))) {
			return ComplexityThreshold::ON;
		} else if (ratios[i - 1] < static_cast<double>(input_sizes[i] * std::log2(input_sizes[i]) * input_sizes[i]) /
		                               (input_sizes[i - 1] * std::log2(input_sizes[i - 1]) * input_sizes[i - 1])) {
			return ComplexityThreshold::ONLOGN;
		} else if (ratios[i - 1] <
		           static_cast<double>(input_sizes[i] * input_sizes[i]) / (input_sizes[i - 1] * input_sizes[i - 1])) {
			return ComplexityThreshold::ON2;
		} else if (ratios[i - 1] < static_cast<double>(input_sizes[i] * input_sizes[i] * input_sizes[i]) /
		                               (input_sizes[i - 1] * input_sizes[i - 1] * input_sizes[i - 1])) {
			return ComplexityThreshold::ON3;
		} else if (ratios[i - 1] < std::pow(2.0, input_sizes[i]) / std::pow(2.0, input_sizes[i - 1])) {
			return ComplexityThreshold::O2N;
		}
	}

	return ComplexityThreshold::OFACT; // Default case
}

ComplexityThreshold measure_time_complexity(const std::function<void(size_t)> & setup,
                                            const std::function<void(size_t)> & lambda,
                                            const std::vector<size_t> & input_sizes, size_t repetitions)
{
	std::vector<double> times;

	for (const auto & input_size : input_sizes) {
		double avg_time = measure_execution_time(setup, lambda, input_size, repetitions);
		times.push_back(avg_time);
	}

	ComplexityThreshold complexity = determine_complexity(times, input_sizes);
	return complexity;
}
