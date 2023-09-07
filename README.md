# The S++ programming language

## Introduction

This document aims at covering the S++ programming language in its purpose, spirit and timeline.  
S++ aims at being a easy-to-use and safe language while not compromising one bit on performance.  
Direct competitors of S++ are first Rust (in its motivation) and then C++ (in market share).

## Overview

The main idea of S++ revolves around maximising data locality through memory-safe patterns. To achieve that, S++ defines a novel concept of the `sequence`.  
A sequence is similar to a `struct` in C, except its size and layout is dynamic at runtime. Members of the sequence can depend of earlier members to dynamically define their size or even presence at all. For example, an array member can have its size defined by an earlier unsigned integer scalar. A payload member can be made optional depending on the state of a bit in an earlier member.

Sanity restrictions are implemented to enforce that undefined behavior can never happen, as long as the execution platform is uncompromised.  
S++ relies heavily on static analysis to infer temporary array dimensions and detect potential undefined behavior.

More importantly, S++ is more of an experimental language where simplicity of the implementation, safety and top performance are equivalent highest-level priorities. Established conventions bear no value in this context. S++ does not aim at being a production-ready language, even if long-term reliability and stability in its design is also a priority. At best, having S++ being influencial in its concepts and spirit would be a reasonable goal.