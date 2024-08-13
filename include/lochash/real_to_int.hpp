#ifndef _INCLUDED_real_to_int_hpp
#define _INCLUDED_real_to_int_hpp

#include <emmintrin.h> // SSE2 intrinsics including _mm_cvtsd_si64
#include <type_traits>


// Check if we're on an x86-64 platform with SSE2 support
#if defined(_M_X64) || defined(__x86_64__)
#define USE_SIMD 1
#else
#define USE_SIMD 0
#endif

/**
 * Converts a floating-point value to an integer using SSE or SSE2 instructions.
 * This function is faster than using std::round or casting to an integer type.
 *
 * @tparam RealType The floating-point type to convert from.
 * @tparam IntType The integer type to convert to.
 * @param value The value to convert.
 * @return The converted integer value.
 */
template <typename RealType, typename IntType>
inline constexpr IntType real_to_int(RealType value)
{
	static_assert(std::is_integral_v<IntType>, "IntType must be an integral type.");

#if USE_SIMD
	if constexpr (std::is_floating_point_v<RealType>) {
		if constexpr (std::is_same_v<RealType, float>) {
			if constexpr (sizeof(IntType) == 4) {
				// Convert float to int32 using SSE
				__m128 val = _mm_set_ss(value); // Load the float into an SSE register
				return _mm_cvtss_si32(val);     // Convert it to int32
			} else {
				// Convert float to int32 using SSE, then cast to smaller or larger type
				__m128  val  = _mm_set_ss(value);
				int32_t temp = _mm_cvtss_si32(val);
				return static_cast<IntType>(temp); // Truncate or extend to other integer sizes
			}
		} else if constexpr (std::is_same_v<RealType, double>) {
			if constexpr (sizeof(IntType) == 4) {
				// Convert double to int32 using SSE2
				__m128d val = _mm_set_sd(value); // Load the double into an SSE2 register
				return _mm_cvtsd_si32(val);      // Convert it to int32
			} else if constexpr (sizeof(IntType) == 8) {
				// Convert double to int64 using SSE2
				return _mm_cvtsd_si64(_mm_set_sd(value));
			} else {
				// Convert double to int32 using SSE2, then cast to smaller or larger type
				__m128d val  = _mm_set_sd(value);
				int32_t temp = _mm_cvtsd_si32(val);
				return static_cast<IntType>(temp); // Truncate or extend to other integer sizes
			}
		}
	} else if constexpr (std::is_integral_v<RealType>) {
		// If RealType is already an integer, simply cast it to the desired IntType
		return static_cast<IntType>(value);
	}
#else
	// Fallback for non-x86/x86-64 platforms or those without SSE2 support
	return static_cast<IntType>(value);
#endif
}
#endif //_INCLUDED_real_to_int_hpp
