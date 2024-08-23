# Location Hash (lochash)

![Build Status](https://github.com/Justin-Randall/lochash/actions/workflows/cmake-multi-platform.yml/badge.svg)
[![codecov](https://codecov.io/gh/Justin-Randall/lochash/graph/badge.svg?token=IYN1GQM2NY)](https://codecov.io/gh/Justin-Randall/lochash)

LocHash is an *n*-dimensional location hash spatial database. This is not a novel concept, but not often utlized.

(Personal note from the author): A fantastic coder, [Calan Thurow](https://www.linkedin.com/in/calan-thurow/), introduced me to this decades ago for optimizing SQL databases to produce shockingly fast queries based on vector-specified proximity. He was decades ahead of our peers in game development. This algorithm applies very well to server-side, web-scale distributed processing, and message routing, where I have frequently leveraged the concept to address $O(n^2)$ interactions that absolutely wreck shipping products when faced with the "rich man's" problem of too many customers.

This implementation is illustrative and by no means perfect. It still relies on std::unordered_map under the hood when there are other implementations that may be faster. Contributions via PRs are welcome!

## Purpose

There are a number of popular spatial database algorithms, from quad-trees and binary space partitions to statically processed potentially visible set (PVS) graphs. This one in particular is most often used for parallel computation, structured queries and distributed processing. It is intended to dramatically *reduce* the number of precision calculations required based on coordinate proximity in some data-space (such as, but not restricted to, location).

## Design

The underlying design is elegantly simple. Coordinates are first quantized to fixed-point integers. The quantization is opinionated and requires a power of 2 step (1, 2, 4, 8, 16, 32, 64 ...) to utilize shift operations, which use a single CPU cycle rather than 29 or more instructions for modulo/division operations.

The storage algorighm utilizes an amortized-constant time unordered map for lookups, insertions and removals. It is optimized for reads and inserts. The implementation uses an std::unordered_map to allow for custom allocators (say, a stack allocation strategy to avoid slower heap and locking, if it all fits ... or a small-block allocator for fixed-size, etc.).

This fits especially well with distributed, back-end mapping, where individual buckets may be associated with event-streaming channels across multiple servers at web-scale so that millions or billions of objects that may potentially interact are localized to a processor by location. Clients (and servers) can make a very fast calculation to determine which broadcast channels to subscribe to.

It also works very well in standalone clients that have lots of objects instantiated, but need to cull potential interactions to other objects in the immediate vicinity -- so a range query returns a handful of buckets, at most, with a few objects in each, reducing the number of precision queries required for things like collision detection or other interactions.

Recursive location databases are also on the menu. 2 or 3 amortized constant-time lookups can handle enormous query spaces. Objects within a few meters may be on a leaf, with the leaf database handling several kilometers, and a parent handling several hundred thousand more of those leaf nodes, etc....

Another advantage of type templating allows integration with other applications. For example, a 3-float coordinate location hash used with UnrealEngine can easily provide a nice helper to convert FVec3 to the triple-float coordinate tuples used by the LocationHash database.

The algorithm quantizes coordinates, first converting to integrals if they are floating point (there are SSE2/SIMD assembly optimizations in place already), then removes precision by some power of 2 before calculating a hash.

So, given a precision of 16 and 3 objects:

```code
Object("A", 24.4f, 20.0f)
Object("B", 30.2f, 18.1f)
Object("C", 50.0f, 40.0f)
```

The hash algorithm converts and strips precision so that A and B are both at 16, 16, while C is at 48, 32.

```code
hash(16, 16) = 1234
hash(48, 32) = 5678
```

(for example).

As far as the hash algorithm is concerned, A and B are equal. C is not.

Grid Representation of Buckets:

```text
+-----+-----+-----+-----+-----+
|     |     |     |     |     |
|     |     |     |     |     |
|     |     |     |     |     |
+-----+-----+-----+-----+-----+
|     |     |     |     |     |
|     |  A  |     |     |     |
|     |  B  |     |     |     |
+-----+-----+-----+-----+-----+
|     |     |     |     |     |
|     |     |     |  C  |     |
|     |     |     |     |     |
+-----+-----+-----+-----+-----+
|     |     |     |     |     |
|     |     |     |     |     |
|     |     |     |     |     |
+-----+-----+-----+-----+-----+
|     |     |     |     |     |
|     |     |     |     |     |
|     |     |     |     |     |
+-----+-----+-----+-----+-----+
```

Of course, the location hash only allocates space where locations are added.

## Visualization in UE5 Editor

This is a brief clip used solely to visualize the algorithm. It is NOT intended to replace a physics system for a 3D game client. There are better ways to do that. This is solely to help visualize what is happening.

There are 2 modes in the visualization: brute-force $O(n^2)$ and leveraging LocHash (amortized constant time lookups). Additionally, there is a visualization showing which objects are checking for collisions with other objects. In the brute force mode, the screen is filled with line draws. Switching to LocHash, there are nearly 0 collision checks with 100 objects in the scene. An additional debug draw for the buckets is also shown, simply to illustrate the point that LocHash is fine for determinig *potentially* interacting objects.

Note 3 data points in the upper left hand of the UI: frame rate, number of objects and number of collision checks.

[![YouTube](http://i.ytimg.com/vi/5gttK_MYwz4/hqdefault.jpg)](https://www.youtube.com/watch?v=5gttK_MYwz4)

For games, this is primarily a server-side optimization where processing and message routing can be distributed across hundreds or thousands (or more) of processes to deal with scale.

## Implementation

The implementation relies heavily on the C++ standard library with modern features. Compiler upgrades for optimizations and features come freely. The generated code is already well-optimized, though performance analysis found some slow conversions, so there is optimized assembly for SSE2 or whatever is available on the target platform.

For example, this library does a LOT of floating point conversion, and even modern compilers generate pretty slow output, so there is an float to integer conversion optimization:

```cpp
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
```

There are similar optimizations for equality comparison operations. These used to turn up in performance analysis (perf, Intel's VTune) and are no longer hot-spots. Most optimizations when using it are really about client code reducing call counts.

Drop-in standard library replacements fine-tuned for specific applications, such as gaming or AI, should work if they are API compliant (allocators, hash functions based on special knowledge of the data being used, etc...).

There is liberal use of static_assert<> to unsnarl the worst of compiler errors with template instantiation output to direct users away from improper usage (things like mixing types for coordinates, for example).

Also note that it has a decent test suite with coverage reporting. If coverage falls below 98%, the build breaks. It is based on [tdd-cmake-template](https://github.com/Justin-Randall/tdd-cmake-template), another project that may be of use for C++ coders starting new projects using TDD and CMake.

## Other Uses

This *n-dimensional* database is also well-suited for semantic maps or many other applications where various coordinates can aggregate to co-locate datapoints on proximity. For example, matchmaking players on similar traits: their skill on an X axis, their ping on a Y axis, their community rating on a Z axis and maybe throw others in such as "community engagement" on a W axis. Since it is *n*-dimensional, it can supplement semantic mapping along various axes. For example, an AI vector database for images can query on some dimension of "dog-ness vs cat-ness" or other traits that emerge in a model, often without understanding which groupings emerge from training. Modern AI can have dozens or hundreds of dimensions in the data, and it is merely a compile-time parameter for this implementation.

## Caveat

This is a *hash*-based algorithm. This means collisions are *possible* unless perfect hashing is used (can be slow in inserts, fast on lookups). The goal is to reduce very large data sets to very small data sets to make fewer, expensive yet precise calculations. There may be *billions* of potentially relevant objects in the data space. LocHash can reduce that to a few or maybe a dozen very quickly and efficiently. It is no guarantee that some irrelevant calculations may be included.

For example, a 128-dimension neural network may produce vectorized results for context. A lochash can query and bucket similar results by semantic relavence. This is algorithm is not restricted to physics and message routing.
