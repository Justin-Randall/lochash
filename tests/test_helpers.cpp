#include "test_helpers.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

std::string to_string(Complexity threshold) {
  // convert enum to string
  switch (threshold) {
  case Complexity::ERROR:
    return "ERROR";
  case Complexity::O1:
    return "O(1)";
  case Complexity::OLogN:
    return "O(log n)";
  case Complexity::ON:
    return "O(n)";
  case Complexity::ONLogN:
    return "O(n log n)";
  case Complexity::ON2:
    return "O(n^2)";
  case Complexity::ON3:
    return "O(n^3)";
  case Complexity::O2N:
    return "O(2^n)";
  case Complexity::ONFactorial:
    return "O(n!)";
  case Complexity::OUnknown:
    return "O(Unknown)";
  default:
    return "Undefined";
  }
}

double measure_execution_time(const std::function<void(size_t)> &setup,
                              const std::function<void(size_t)> &lambda,
                              size_t input_size, size_t repetitions) {
  using namespace std::chrono;
  double total_time = 0.0;

  for (int i = 0; i < repetitions; ++i) {
    setup(input_size);

    auto start = high_resolution_clock::now();
    lambda(input_size);
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    total_time += elapsed.count();
  }

  double average_time = total_time / repetitions;
  return average_time;
}

// Helper function to perform linear regression and return R^2 value
double linear_regression(const std::vector<double> &x,
                         const std::vector<double> &y) {
  size_t n = x.size();
  if (n != y.size() || n == 0)
    return -std::numeric_limits<double>::infinity();

  double meanX = std::accumulate(x.begin(), x.end(), 0.0) / n;
  double meanY = std::accumulate(y.begin(), y.end(), 0.0) / n;

  double ssTotal = 0;
  double ssResidual = 0;
  double slope = 0;
  double intercept = 0;
  bool validSlope = false;

  double num = 0;
  double den = 0;
  for (size_t i = 0; i < n; ++i) {
    double dx = x[i] - meanX;
    num += dx * (y[i] - meanY);
    den += dx * dx;
  }
  if (den != 0) {
    slope = num / den;
    intercept = meanY - slope * meanX;
    validSlope = true;
  }

  for (size_t i = 0; i < n; ++i) {
    double yPred = intercept + slope * x[i];
    ssTotal += std::pow(y[i] - meanY, 2);
    ssResidual += std::pow(y[i] - yPred, 2);
  }

  double r2 = 1 - ssResidual / ssTotal;
  if (!validSlope)
    r2 = -std::numeric_limits<double>::infinity();
  if (ssTotal == 0)
    r2 = 1; // Perfect fit case
  return r2;
}

Complexity determine_complexity(const std::vector<size_t> &input_sizes,
                                const std::vector<double> &times) {
  if (input_sizes.size() != times.size() || input_sizes.empty()) {
    return Complexity::ERROR;
  }

  std::vector<double> input_sizes_double(input_sizes.size());
  std::transform(input_sizes.begin(), input_sizes.end(),
                 input_sizes_double.begin(),
                 [](size_t val) { return static_cast<double>(val); });

  std::vector<std::pair<double, Complexity>> fits;

  // O(1)
  fits.emplace_back(
      linear_regression(std::vector<double>(input_sizes.size(), 1.0), times),
      Complexity::O1);

  // O(log n)
  std::vector<double> log_input_sizes;
  bool validLogX = true;
  for (double val : input_sizes_double) {
    if (val <= 0) {
      validLogX = false;
      break;
    }
    log_input_sizes.push_back(std::log2(val));
  }
  if (validLogX) {
    fits.emplace_back(linear_regression(log_input_sizes, times),
                      Complexity::OLogN);
  } else {
    fits.emplace_back(-std::numeric_limits<double>::infinity(),
                      Complexity::OLogN);
  }

  // O(n)
  fits.emplace_back(linear_regression(input_sizes_double, times),
                    Complexity::ON);

  // O(n log n)
  std::vector<double> n_log_n_times(input_sizes_double.size());
  if (validLogX) {
    std::transform(input_sizes_double.begin(), input_sizes_double.end(),
                   n_log_n_times.begin(),
                   [](double val) { return val * std::log2(val); });
    fits.emplace_back(linear_regression(n_log_n_times, times),
                      Complexity::ONLogN);
  } else {
    fits.emplace_back(-std::numeric_limits<double>::infinity(),
                      Complexity::ONLogN);
  }

  // O(n^2)
  std::vector<double> squared_input_sizes(input_sizes_double.size());
  std::transform(input_sizes_double.begin(), input_sizes_double.end(),
                 squared_input_sizes.begin(),
                 [](double val) { return val * val; });
  fits.emplace_back(linear_regression(squared_input_sizes, times),
                    Complexity::ON2);

  // O(n^3)
  std::vector<double> cubed_input_sizes(input_sizes_double.size());
  std::transform(input_sizes_double.begin(), input_sizes_double.end(),
                 cubed_input_sizes.begin(),
                 [](double val) { return val * val * val; });
  fits.emplace_back(linear_regression(cubed_input_sizes, times),
                    Complexity::ON3);

  // O(2^n)
  std::vector<double> exp_input_sizes(input_sizes_double.size());
  std::transform(input_sizes_double.begin(), input_sizes_double.end(),
                 exp_input_sizes.begin(),
                 [](double val) { return std::pow(2, val); });
  fits.emplace_back(linear_regression(exp_input_sizes, times), Complexity::O2N);

  // O(n!)
  std::vector<double> fact_input_sizes(input_sizes_double.size());
  std::transform(input_sizes_double.begin(), input_sizes_double.end(),
                 fact_input_sizes.begin(),
                 [](double val) { return std::tgamma(val + 1); });
  fits.emplace_back(linear_regression(fact_input_sizes, times),
                    Complexity::ONFactorial);

  auto best_fit = std::max_element(
      fits.begin(), fits.end(),
      [](const std::pair<double, Complexity> &a,
         const std::pair<double, Complexity> &b) {
        if ((std::isnan(a.first) ||
             a.first == -std::numeric_limits<double>::infinity()) &&
            (std::isnan(b.first) ||
             b.first == -std::numeric_limits<double>::infinity())) {
          return false; // Prevent invalid comparator
        }
        if (std::isnan(a.first) ||
            a.first == -std::numeric_limits<double>::infinity())
          return true;
        if (std::isnan(b.first) ||
            b.first == -std::numeric_limits<double>::infinity())
          return false;
        return a.first < b.first;
      });

  if (std::isnan(best_fit->first) ||
      best_fit->first == -std::numeric_limits<double>::infinity()) {
    return Complexity::OUnknown;
  }

  return best_fit->second;
}

Complexity
measure_time_complexity(const std::function<void(size_t input_size)> &setup,
                        const std::function<void(size_t input_size)> &lambda,
                        const std::vector<size_t> &input_sizes,
                        size_t repetitions) {
  std::vector<double> times;

  // Take three measurements, record the lowest time and return that.
  Complexity bestComplexity = Complexity::OUnknown;
  for (size_t i = 0; i < 3; ++i) {
    for (const auto &input_size : input_sizes) {
      double avg_time =
          measure_execution_time(setup, lambda, input_size, repetitions);
      times.push_back(avg_time);
    }
    Complexity complexity = determine_complexity(input_sizes, times);
    if (complexity < bestComplexity) {
      bestComplexity = complexity;
    }
  }

  return bestComplexity;
}
