#include "gtest/gtest.h"
#include <initializer_list>

enum class ComplexityThreshold { O1, ON, OLOGN, ON2, ON3 };
void test_complexity(std::initializer_list<std::size_t> counts, std::initializer_list<long long> timings,
                     ComplexityThreshold threshold);
