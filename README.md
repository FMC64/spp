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

// Signed two's complement fixed-point scalar with bit count `IntegerBitCount + FractionalBitCount`
template <host_unsigned IntegerBitCount, host_unsigned FractionalBitCount>
class fixed_signed;

// Unsigned fixed-point scalar with bit count `IntegerBitCount + FractionalBitCount`
template <host_unsigned IntegerBitCount, host_unsigned FractionalBitCount>
class fixed_unsigned;

// IEEE 754 float with bit count `N`
template <host_unsigned N>
class ieee754_float, float;

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
// Under a 64-bit system (as AMD64, ARM64, and so on), this is equivalent to `float<64>`
// Under a 32-bit system (as IA-32, ARMv7, and so on), this is equivalent to `float<32>`
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


#### Scalar literals

Here S++ shines with its unconventionality. Because of the implementation simplicity requirement, scalar literals are represented in little-endian notation.  
Little-endian is the standard on a S++ system. Conventional big-endian notation (denoted `arabic` here) is optional but not recommended for optimal compiling speed. The early versions of S++ will not include a `arabic` literal parser.

Because little-endian is basically not ever used for human interfaces, their usage must be prepended by a `*` symbol. For example, the arabic `10` decimal scalar for the number ten is represented by `*01` as standard decimal in S++, while both of them hold the exact same value. Both `arabic` and default notations are supported. By default, S++ standard functions output in little-endian decimal while always explicitly specifying the `*` prefix symbol.

Honestly, it is tempting to go all the way in there and not support `arabic` notation altogether. That would be in the spirit of the language. But that would make the language much less attractive to beginners so both options are redundently available.

As for the literals themselves, they have their own lossless unnamed type, just wide enough to represent the value with full precision. They can only be represented on the host during compile-time. Lossless compile-time operations involving multiple literals is possible. They may need to be `lossy` converted to the target type if not representable in binary.  
Futhermore, literals are in two sets: binary fixed-point unsigned integers and reals, the former being a subset of the latter. Bitwise operations can only be operated on binary fixed-point unsigned integers.

Let's explore how to express a literal:

The general notation is `*([BASE])([FRACTION].)[INTEGER](x[EXPONENT])`, where:
- `([BASE])` is decimal if not supplied, or may be `x` for hexadecimal notation, `o` for octal, `q` for base-4 and `b` for binary
- `([FRACTION].)` is the fractional part of the literal if the literal needs to be fractional, represented in the selected base. As this is little-endian, the fractional part starts off with the least significant digit to end with the digit right under the point in weight. There is no limit to the size of this part.
- `[INTEGER]` is the mandatory section of the literal, represented in the selected base. Starts with the least significant digit to end with the most significant digit. There is no limit to the size of this part.
- `(x[EXPONENT])` is the optional exponent of the overall literal, which defaults to zero. The conventional `e` is not used, as it is reserved for hexadecimal. The exponent follows the selected base. Useful to represent large numbers without any trouble.

In `arabic` notation, the general notation is `([BASE])INTEGER(.[FRACTION])(x[EXPONENT])`.

The value of the literal is `(INTEGER + FRACTION * pow(BASE, -FRACTION_WIDTH)) * pow(BASE, EXPONENT)`, where `FRACTION_WIDTH` is the number of digits in `BASE` within `FRACTION`.  
The compile-time literal system should support complex arithmetic operations such as exponential and trigonometric ones (`sqrt`, `pow`, `log<Base>`, `exp`, `ln`, `cos`, `sin`, `tan`, `arccos`, `arcsin` and `arctan`). Real constants with their exact value such as `pi`, `tau` and `euler` are available. To guarantee no loss, the literal must be lazily evaluated when converted to a builtin non-literal data type.

#### String literals

S++ declares string literals similarly to C++, using double quotes. Example: `"Hello world!"`. This results into a compile-time array of the current character scalar. An encoding keyword can be prepended to the string literal to change its encoding from the current one.

The default encoding is UTF-8 and will produce arrays of `unsigned<8>` (or equivalently, `u8`). The encoding in the current module can be changed using the compile-time command `default_encoding([ENCODING_KEYWORD])`. The full list of available encodings is:
- `encoding_utf8`: UTF-8, will yield an array of `unsigned<8>` wrapped into a `StringUtf8` object. Access to this object yields `unsigned<32>` code points.
- `encoding_utf16`: UTF-16, will yield an array of `unsigned<16>` wrapped into a `StringUtf16` object. Access to this object yields `unsigned<16>` code points.
- `encoding_ucs2`: UCS-2, will yield an array of `unsigned<16>` wrapped into a `StringUtf16` object. Fixed-size characters, the code points are presented as-is in the array.
- `ascii`: ASCII encoding, will yield an array of `unsigned<8>` wrapped into a `StringAscii` object. Only the ASCII table can be encoded within the literal. Fixed-size characters, the code points are presented as-is in the array.