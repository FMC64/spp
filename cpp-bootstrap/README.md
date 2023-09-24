# S++ implementation in C++

This is a C++ implementation of S++.

Obviously S++ needs to self-compile at some point, but to start off we are doing the first version of the compiler in C++.  
Then it will be rewritten entirely in S++. First compiled using this program, then set to self-build!

This first version is designed to be quick to iterate on for C++ standards. Nothing really optimized, just plain modern C++ not too hard to read. The S++ implementation will have not much in common with the C++ one, so there is nothing to worry about in terms of making something long-term.

This C++ version is more like an interpreter. Everything that can be compile-time evaluated gets compile-time evaluated, then the rest of the runtime program gets executed based on a high-level bytecode. The goal is just to be able to run S++ programs and ultimately build a S++ implementation of S++, so it doesn't need to be fast. We'll deal with machine-specific code with the long-term S++ implementation.

This implementation has a significant bias in using C I/Os to make builds fast.

## Building

Use `make`, with `$CXX` being a C++23-capable compiler with GCC interface.

## Running

`./s++ path/to/entrypoint.spp` will run the S++ source being supplied.

`./s++ path/to/entrypoint.spp --inspect` will not run the source, only reprint the unrolled bytecode with extensive type and value annotations.