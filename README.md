# Location Hash (lochash)

LocHash is an *n*-dimensional location hash spatial database.

## Purpose

There are a number of popular spatial database algorithms, from quad-trees and binary space partitions to statically processed potentially visible set (PVS) graphs. This one in particular is most often used for parallel computation, structured query and distributed processing.

## Design

The underlying design is elegantly simple. Coordinates are first quantized to native word-size, fixed-point integers. The quantization is opinionated and requires a power of 2 step (1, 2, 4, 8, 16, 32, 64 ...) to utilize shift operations to use a single CPU cycle rather than 29 or more for modulo/division operations.

The storage algorighm utilizes an amortized-constant time unordered map for lookups, insertions and removals. It is optimized for reads and inserts. The implementation uses an std::unordered_map to allow for custom allocators (say, a stack allocation strategy to avoid slower heap and locking, if it all fits ... or a small-block allocator for fixed-size, etc.).

This maps especially well to distributed, back-end mapping, where individual buckets may be associated with event-streaming channels across multiple servers at web-scale so that millions or billions of objects that may potentially interact are localized to a processor by location. Clients (and servers) can make a very fast calculation to determine which broadcast channels to subscribe to.

It also works very well in standalone clients that have millions of objects instantiated, but need to cull potential interactions to other objects in the immediate vacinity -- so a range query returns a handful of buckets, at most, with a few objects in each, reducing the number of precision queries required for things like collision detection or other interactions.

The use of templates also allows for recursive location databases. 2 or 3 amortized constant-time lookups can handle enormous query spaces. Objects within a few meters may be on a leaf, with the leaf database handling several hundred thousand kilometers, and a parent handling several hundred thousand more of those leaf nodes, etc....

Another advantage of type templating allows integration with other applications. For example, a 3-float coordinate location hash used with UnrealEngine can easily provide a nice helper to convert FVec3 to the triple-float coordinate tuples used by the LocationHash database.

## Implementation

The implementation relies heavily on the C++ standard library with modern features. Compiler upgrades for optimizations and features come freely. The generated code is already well optimized. Drop-in standard library replacements fine-tuned for specific applications, such as gaming or AI, should work if they are API compliant.

There is liberal use of static_assert<> to unsnarl the worst of compiler errors with template instantiation output to direct users away from improper usage (things like mixing types for coordinates, for example).

## Performance

(TODO, need to write a perf test suite)
