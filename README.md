# The S++ programming language

## Introduction

This document aims at covering the S++ programming language in its purpose, spirit and timeline.  
S++ aims at being a easy-to-use and safe language while not compromising one bit on performance.  
Direct competitors of S++ are first Rust (in its motivation) and then C++ (in the high-performance market).

S++ is designed to be comparable to JavaScript and Python in its absolute approachability, from a complete programming newcomer perspective. A newbie should have no more trouble getting a simple algorithm to work on dynamic languages than on S++. In this regard, S++ competition is potentially as large as the entire active market of programming languages. Its performance combined to easy-to-use approach and safety would make S++ a very competitive general-purpose programming language. That being said, S++ also is admittedly unconventional and would represent a higher relative barrier to entry to seasoned engineers.

S++ takes mainly inspiration from TypeScript, Ruby and C++ and is under ongoing pre-alpha development. S++ is designed to be very fast to build, it can be used as a scripting language.

## Overview

The main idea of S++ revolves around maximising data locality through memory-safe patterns. To achieve that, S++ defines a novel concept of the `sequence`.  
A sequence is similar to a `struct` in C, except its size and layout is dynamic at runtime. Members of the sequence can depend of earlier members to dynamically define their size or even presence at all. For example, an array member can have its size defined by an earlier unsigned integer scalar. A payload member can be made optional depending on the state of a bit in an earlier member.

Sanity restrictions are implemented to enforce that undefined behavior can never happen, as long as the execution platform is uncompromised.  
S++ relies heavily on static analysis to infer temporary array dimensions and detect potential undefined behavior.

More importantly, S++ is more of an experimental language where simplicity of the implementation, safety and top performance are equivalent highest-level priorities. Established conventions bear no value in this context. S++ does not aim at being a production-ready language, even if long-term reliability and stability in its design is also a priority. At best, having S++ being influencial in its concepts and spirit would be a reasonable goal.

## Concepts and interface

### Builtin data types

S++ agrees with C++ that compile-time computations and metaprogramming are powerful features. The user should be able to generate specialized code at compile-time, at the cost of increased binary size. S++ does not follow the more contemporary approach of dynamic languages of using runtime polymorphism based off a single set of generic functions and methods in the binary, as Java (as JVM-based languages such as Scala and Dart) and JavaScript (including TypeScript) do.

In this way, S++ is heavily templated and expects the user to use them. Even builtin data types are in fact templates to remove redundancy and improve elegance in its usage.

#### Definitions

```spp
// Boolean type
class bool;

// Signed two's complement integer with bit count `N`
template <host_unsigned N>
class signed;

// Unsigned integer with bit count `N`
template <host_unsigned N>
class unsigned;

// Signed two's complement fixed-point scalar with bit count `IntegerBitCount + FractionalBitCount`,
// where `IntegerBitCount` must be at least one to store the sign
template <host_unsigned IntegerBitCount, host_unsigned FractionalBitCount>
class fixed_signed;

// Unsigned fixed-point scalar with bit count `IntegerBitCount + FractionalBitCount`
template <host_unsigned IntegerBitCount, host_unsigned FractionalBitCount>
class fixed_unsigned;

// IEEE 754 float with bit count `N`
template <host_unsigned N>
class ieee754_float;

// General-purpose float
template <bool IsSigned, host_unsigned ExponentSize, host_unsigned MantissaSize, host_unsigned ExponentBias>
class minifloat;
```

The semicolon is optional in most circumstances within use of the language. Here, it denotes a declaration (not definition) of the class, similarly to C++. Objects declared without a definition cannot be exported out of a module. Here, these objects are declared & defined intrisically within the language.

All of these builtin data types may be inherited from. In addition, S++ presents commonly used data types, builtin as well.

```spp
// Unsigned integer of the native register size of the platform the S++ compiler runs on
// Under a 64-bit system (as AMD64, ARM64, and so on), this is equivalent to `unsigned<64>`
// Under a 32-bit system (as IA-32, ARMv7, and so on), this is equivalent to `unsigned<32>`
// This is guaranteed to be wide enough to hold a pointer represented as an integer.
native class host_unsigned, host_u;

// Signed two's complement integer of the native register size of the platform the S++ compiler runs on
// Under a 64-bit system (as AMD64, ARM64, and so on), this is equivalent to `signed<64>`
// Under a 32-bit system (as IA-32, ARMv7, and so on), this is equivalent to `signed<32>`
native class host_signed, host_s;

// IEEE 754 float of the native register size of the platform the S++ compiler runs on
// Under a 64-bit system (as AMD64, ARM64, and so on), this is equivalent to `signed<64>`
// Under a 32-bit system (as IA-32, ARMv7, and so on), this is equivalent to `signed<32>`
native class host_float, host_f;

// Similar to `host_unsigned`, `host_signed` and `host_device` respectively,
// these types target the system the application is being built for (and not being built on)
native class device_unsigned, dev_u;
native class device_signed, dev_s;
native class device_float, dev_f;

// Equivalent to `signed` with the number in the identifier being `N`
class s8, s16, s32, s64, s128, s256;
// Equivalent to `unsigned` with the number in the identifier being `N`
class u8, u16, u32, u64, u128, u256;

// Equivalent to `ieee754_float` with the number in the identifier being `N`
// `f80` is the non-IEEE 754, x86 extended precision format
// All others are IEEE 754 standard
class f16, f32, f64, f80, f128, f256;
```

By default, S++ presents every value of these data types to every application, without any restriction. Non-natively supported data types are emulated in software at an increased runtime cost. There is an understanding that the user would go on and emulate these data types anyway, so S++ builts them in to present increased robustness.

Because many users will want to use non-software emulated data types exclusively, some keywords are introduced to issue a warning when software emulation would be necessary.  The `native` keywords hints that the following data type must be supported in hardware in its loading and storing. Warnings will be issued if a particular non-hardware supported operation is performed. Other `native`-based keywords are available:
- `native_load_store`: more relaxed version of `native`. Takes precedence over `native`. Only loading and storing of the data type must be supported in hardware. All arithmetic operations are allowed to be performed in software if not available in hardware.
- `non_native`: takes precedence over every other `native` keyword. Allows every software trick to emulate the data type, without issuing any warning.

By default, the host and device-native data types are denoted as `native`. A `native`-based keyword can be prepended to every data type as default in the current module using the compile-time command: `default_native([NATIVE_KEYWORD])`. This cannot be changed for required modules unless modifying them.

#### Type conversions

S++ presents two operators to be combined to realise conversions between two different data types:
- `[TARGET_DATA_TYPE]([SOURCE_VALUE])`: lossless data conversion. `[SOURCE_VALUE]` must be entirely representable within `[TARGET_DATA_TYPE]`. This is the indended way of performing sign extension for two's complement integers. This is the default data conversion within the language, other types of conversion need to be explicitely called.
- `[TARGET_DATA_TYPE] lossy ([SOURCE_VALUE])`: lossy data conversion. `[TARGET_DATA_TYPE]` must be able to represent at least the sign of `[SOURCE_VALUE]`. A lossy conversion is allowed to truncate bits of the source that do not fit into the target data type.
- `[TARGET_DATA_TYPE] reinterpret ([SOURCE_VALUE])`: reinterpreted data conversion. `[TARGET_DATA_TYPE]` must be at least as wide in bits as `[SOURCE_VALUE]`. Bits will be taken as-it from the LSB of the source to the LSB of the target and the rest zero-padded.
- `[TARGET_DATA_TYPE] lossy reinterpret ([SOURCE_VALUE])`: reinterpreted lossy data conversion. The only requiment is that `[TARGET_DATA_TYPE]` and `[SOURCE_VALUE]` must both be scalars. Only `min(bits_of([TARGET_DATA_TYPE]), bits_of([SOURCE_DATA_TYPE])` will be taken from the LSB of the source to the LSB of the target, and the rest zero-padded.

From these definitions, the user will observe that even a `lossy` conversion will not be able to convert a signed data type to an unsigned one. There is then a pratical bias in `lossy` converting an unsigned scalar to a signed one when comparisons or accumulations between different data types are necessary.