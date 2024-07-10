# Location Hash (lochash)

![Build Status](https://github.com/Justin-Randall/lochash/actions/workflows/cmake-multi-platform.yml/badge.svg)
[![codecov](https://codecov.io/gh/Justin-Randall/lochash/graph/badge.svg?token=IYN1GQM2NY)](https://codecov.io/gh/Justin-Randall/lochash)

LocHash is an *n*-dimensional location hash spatial database.

## Purpose

There are a number of popular spatial database algorithms, from quad-trees and binary space partitions to statically processed potentially visible set (PVS) graphs. This one in particular is most often used for parallel computation, structured queries and distributed processing. It is intended to dramatically *reduce* the number of precision calculations required based on coordinate proximity in some data-space (such as, but not restricted to, location).

## Design

The underlying design is elegantly simple. Coordinates are first quantized to native word-size, fixed-point integers. The quantization is opinionated and requires a power of 2 step (1, 2, 4, 8, 16, 32, 64 ...) to utilize shift operations to use a single CPU cycle rather than 29 or more for modulo/division operations.

The storage algorighm utilizes an amortized-constant time unordered map for lookups, insertions and removals. It is optimized for reads and inserts. The implementation uses an std::unordered_map to allow for custom allocators (say, a stack allocation strategy to avoid slower heap and locking, if it all fits ... or a small-block allocator for fixed-size, etc.).

This fits especially well with distributed, back-end mapping, where individual buckets may be associated with event-streaming channels across multiple servers at web-scale so that millions or billions of objects that may potentially interact are localized to a processor by location. Clients (and servers) can make a very fast calculation to determine which broadcast channels to subscribe to.

It also works very well in standalone clients that have millions of objects instantiated, but need to cull potential interactions to other objects in the immediate vacinity -- so a range query returns a handful of buckets, at most, with a few objects in each, reducing the number of precision queries required for things like collision detection or other interactions.

The use of templates also allows for recursive location databases. 2 or 3 amortized constant-time lookups can handle enormous query spaces. Objects within a few meters may be on a leaf, with the leaf database handling several hundred thousand kilometers, and a parent handling several hundred thousand more of those leaf nodes, etc....

Another advantage of type templating allows integration with other applications. For example, a 3-float coordinate location hash used with UnrealEngine can easily provide a nice helper to convert FVec3 to the triple-float coordinate tuples used by the LocationHash database.

The algorithm quantizes coordinates, first converting to native word size (size_t) if they are floating point, then removes precision by some power of 2 before calculating a hash.

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

## Implementation

The implementation relies heavily on the C++ standard library with modern features. Compiler upgrades for optimizations and features come freely. The generated code is already well optimized. Drop-in standard library replacements for fine-tuned for specific applications, such as gaming or AI, should work if they are API compliant.

There is liberal use of static_assert<> to unsnarl the worst of compiler errors with template instantiation output to direct users away from improper usage (things like mixing types for coordinates, for example).

## Other Uses

This *n-dimensional* database is also well-suited for semantic maps or many other applications where various coordinates can aggregate to co-locate datapoints on proximity. For example, matchmaking players on similar traits: their skill on an X axis, their ping on a Y axis, their community rating on a Z axis and maybe throw others in such as "community engagement" on a W axis ... and so on, and so on.

## Caveat

This is a *hash*-based algorith. This means collisions are *possible*. The goal is to reduce very large data sets to very small data sets to make fewer, expensive yet precise calculations. There may be 10 *trillion* potentially relavent objects in the data space. Lochash can reduce that to a few or maybe a dozen very quickly and efficiently. It is no garauntee that some irrelevant calculations may be included.

For example, a 128-dimension neural network may produce vectorized results for context. A lochash can query and bucket similar results by semantic relavence. This is algorithm is not restricted to physics and message routing.
