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

## Specification

### 1. Builtin data types

S++ agrees with C++ that compile-time computations and metaprogramming are powerful features. The user should be able to generate specialized code at compile-time, at the cost of increased binary size. S++ does not follow the more contemporary approach of dynamic languages of using runtime polymorphism based off a single set of generic functions and methods in the binary, as Java (as JVM-based languages such as Scala and Dart) and JavaScript (including TypeScript) do.

In this way, S++ is heavily templated and expects the user to use them. Even builtin data types are in fact templates to remove redundancy and improve elegance in its usage.

#### 1.1 Scalar data types

```spp
// Abstract scalar, cannot be instanciated but can be used as a type parameter
// to accept values which implement such interface
interface scalar;

// Abstract natural integer, encompasses any integer which can represent
// zero and strictly positive integers, but not strictly negative ones.
interface natural extends scalar;

// Abstract signed integer, encompasses any integer including the ones which
// can represent zero, positive and negative values.
interface integer extends natural;

// Abstract real number, can represent any positive number with a finite representation,
// is not required to present an absolute precision on any value. Negative values cannot
// be represented by this type.
interface positive_real extends scalar;

// Abstract real number, can represent any positive or negative number with a finite representation,
// is not required to present an absolute precision on any value.
interface real extends positive_real;

// Abstract transcendental number. Because of their nature, they have their own class of `scalar`s
// as they really cannot be treated as anything other than a symbol with special properties.
interface transcendental extends scalar;

// Boolean type
class bool implements scalar;

// Signed two's complement integer with bit count `N`
template <host_unsigned N>
class signed implements integer;

// Unsigned integer with bit count `N`
template <host_unsigned N>
class unsigned implements natural;

// Signed two's complement fixed-point scalar with bit count `IntegerBitCount + FractionalBitCount`
template <host_unsigned IntegerBitCount, host_unsigned FractionalBitCount>
class fixed_signed implements real;

// Unsigned fixed-point scalar with bit count `IntegerBitCount + FractionalBitCount`
template <host_unsigned IntegerBitCount, host_unsigned FractionalBitCount>
class fixed_unsigned implements positive_real;

// IEEE 754 float with bit count `N`
template <host_unsigned N>
class ieee754_float implements real, float implements real;

// Used by `minifloat`
template <host_unsigned ExponentSize, host_unsigned MantissaSize, host_unsigned ExponentBias>
class unsigned_minifloat implements positive_real;

// Used by `minifloat`
template <host_unsigned ExponentSize, host_unsigned MantissaSize, host_unsigned ExponentBias>
class signed_minifloat implements real;

// General-purpose float
template <bool IsSigned, host_unsigned ExponentSize, host_unsigned MantissaSize, host_unsigned ExponentBias>
using minifloat = IsSigned ?
	unsigned_minifloat<ExponentSize, MantissaSize, ExponentBias> :
	signed_minifloat<ExponentSize, MantissaSize, ExponentBias>;
```

The semicolon is optional in most circumstances within use of the language. Here, it denotes a declaration (not definition) of the class, similarly to C++. Objects declared without a definition cannot be exported out of a module. Here, these objects are declared & defined intrisically within the language.

All of these builtin data types may be inherited from. In addition, S++ presents commonly used scalar data types, builtin as well.

```spp
// Unsigned integer of the native register size of the platform the S++ compiler runs on
// Under a 64-bit system (as AMD64, ARM64, and so on), this is equivalent to `unsigned<64>`
// Under a 32-bit system (as IA-32, ARMv7, and so on), this is equivalent to `unsigned<32>`
// This is guaranteed to be wide enough to hold a pointer represented as an integer.
native class host_unsigned implements natural, host_u implements natural;

// Signed two's complement integer of the native register size of the platform the S++ compiler runs on
// Under a 64-bit system (as AMD64, ARM64, and so on), this is equivalent to `signed<64>`
// Under a 32-bit system (as IA-32, ARMv7, and so on), this is equivalent to `signed<32>`
native class host_signed implements integer, host_s implements integer;

// IEEE 754 float of the native register size of the platform the S++ compiler runs on
// Under a 64-bit system (as AMD64, ARM64, and so on), this is equivalent to `float<64>`
// Under a 32-bit system (as IA-32, ARMv7, and so on), this is equivalent to `float<32>`
native class host_float implements real, host_f implements real;

// Similar to `host_unsigned`, `host_signed` and `host_device` respectively,
// these types target the system the application is being built for (and not being built on)
native class device_unsigned implements natural, dev_u implements natural;
native class device_signed implements integer, dev_s implements integer;
native class device_float implements real, dev_f implements real;

// Equivalent to `signed` with the number in the identifier being `N`
// Each of these `implement integer` (not included for readibility)
class s8, s16, s32, s64, s128, s256;
// Equivalent to `unsigned` with the number in the identifier being `N`
// Each of these `implement natural` (not included for readibility)
class u8, u16, u32, u64, u128, u256;

// Equivalent to `ieee754_float` with the number in the identifier being `N`
// `f80` is the non-IEEE 754, x86 extended precision format
// All others are IEEE 754 standard
// Each of these `implement real` (not included for readibility)
class f16, f32, f64, f80, f128, f256;
```

By default, S++ presents every value of these data types to every application, without any restriction. Non-natively supported data types are emulated in software at an increased runtime cost. There is an understanding that the user would go on and emulate these data types anyway, so S++ builts them in to present increased robustness.

Because many users will want to use non-software emulated data types exclusively, some keywords are introduced to issue a warning when software emulation would be necessary.  The `native` keywords hints that the following data type must be supported in hardware in its loading and storing. Warnings will be issued if a particular non-hardware supported operation is performed. Other `native`-based keywords are available:
- `native_load_store`: more relaxed version of `native`. Takes precedence over `native`. Only loading and storing of the data type must be supported in hardware. All arithmetic operations are allowed to be performed in software if not available in hardware.
- `non_native`: takes precedence over every other `native` keyword. Allows every software trick to emulate the data type if not available in hardware, without issuing any warning.

By default, the host and device-native data types are denoted as `native`. A `native`-based keyword can be prepended to every data type as default in the current module using the compile-time command: `default_native([NATIVE_KEYWORD])`. This cannot be changed for required modules unless modifying them.

#### 1.2. Type conversions

S++ presents two operators to be combined to realise conversions between two different data types:
- `[TARGET_DATA_TYPE]([SOURCE_VALUE])`: lossless data conversion. `[SOURCE_VALUE]` must be entirely representable within `[TARGET_DATA_TYPE]`. This is the indended way of performing sign extension for two's complement integers. This is the default data conversion within the language, other types of conversion need to be explicitely called.
- `[TARGET_DATA_TYPE] lossy ([SOURCE_VALUE])`: lossy data conversion. `[TARGET_DATA_TYPE]` must be able to represent at least the sign of `[SOURCE_VALUE]`. A lossy conversion is allowed to truncate bits of the source that do not fit into the target data type.
- `[TARGET_DATA_TYPE] reinterpret ([SOURCE_VALUE])`: reinterpreted data conversion. `[TARGET_DATA_TYPE]` must be at least as wide in bits as `[SOURCE_VALUE]`. Bits will be taken as-is from the LSB of the source to the LSB of the target and the rest zero-padded.
- `[TARGET_DATA_TYPE] lossy reinterpret ([SOURCE_VALUE])`: reinterpreted lossy data conversion. The only requiment is that `[TARGET_DATA_TYPE]` and `[SOURCE_VALUE]` must both be scalars. Only `min(bits_of([TARGET_DATA_TYPE]), bits_of([SOURCE_DATA_TYPE])` will be taken from the LSB of the source to the LSB of the target, and the rest zero-padded.

From these definitions, the user will observe that even a `lossy` conversion will not be able to convert a signed data type to an unsigned one. There is then a pratical bias in `lossy` converting an unsigned scalar to a signed one when comparisons or accumulations between different data types are necessary.


#### 1.3. Scalar literals

Here S++ shines with its unconventionality. Because of the implementation simplicity requirement, scalar literals are represented in little-endian notation.  
Little-endian is the standard on a S++ system. Conventional big-endian notation (denoted `arabic` here) is optional but not recommended for optimal compiling speed. The early versions of S++ will not include a `arabic` literal parser.

Because little-endian is basically not ever used for human interfaces, their usage must be prepended by a `*` symbol. For example, the arabic `10` decimal scalar for the number ten is represented by `*01` as standard decimal in S++, while both of them hold the exact same value. Both `arabic` and default notations are supported. By default, S++ standard functions output in little-endian decimal while always explicitly specifying the `*` prefix symbol.

Honestly, it is tempting to go all the way in there and not support `arabic` notation altogether. That would be in the spirit of the language. But that would make the language much less attractive to beginners so both options are redundently available.

As for the literals themselves, they have their own lossless unnamed type, just wide enough to represent the value with full precision. They can only be represented on the host during compile-time. Lossless compile-time operations involving multiple literals is possible. They may need to be `lossy` converted to the target type if not representable in binary.  
Futhermore, literals are in two sets: binary fixed-point unsigned integers and positive reals, the former being a subset of the latter. Bitwise operations can only be operated on binary fixed-point unsigned integers.

Let's explore how to express a positive literal (a negative one is defined by using the prefix `-` unary operator):

The general notation is `*([BASE])([FRACTION].)[INTEGER](x[EXPONENT])`, where:
- `([BASE])` is decimal if not supplied, or may be `0h` for hexadecimal notation, `0o` for octal, `0q` for base-4 and `0b` for binary
- `([FRACTION].)` is the fractional part of the literal if the literal needs to be fractional, represented in the selected base. As this is little-endian, the fractional part starts off with the least significant digit to end with the digit right under the point in weight. There is no limit to the size of this part.
- `[INTEGER]` is the mandatory section of the literal, represented in the selected base. Starts with the least significant digit to end with the most significant digit. There is no limit to the size of this part but must feature at least a single digit.
- `(x[EXPONENT])` is the optional exponent of the overall literal, which defaults to zero. The conventional `e` is not used, as it is reserved for hexadecimal (you read that right, you can exponentiate hexadecimal literals). The exponent follows the selected base. Useful to represent large numbers without any trouble.

In `arabic` notation, the general notation is `([BASE])INTEGER(.[FRACTION])(x[EXPONENT])`.

Note that each digit sequence can contain a `_` which will get ignored while parsing. These characters can be inserted to increase readability of the literal.

The value of the literal (which is also the internal representation) is `(INTEGER + FRACTION * pow(BASE, -FRACTION_WIDTH)) * pow(BASE, EXPONENT)`, where `FRACTION_WIDTH` is the number of digits in `BASE` within `FRACTION`. All of the individual components are `natural`s, but the overall value may be promoted to a `positive_real` by evaluating it.  
The compile-time literal system should support complex arithmetic operations such as exponential and trigonometric ones (`sqrt`, `pow`, `log<Base>`, `exp`, `ln`, `cos`, `sin`, `tan`, `arccos`, `arcsin` and `arctan`).  
`transcendental` constants with their exact value such as `pi`, `tau` and `e` are available. Scalar literals are guaranteed to be lossless, even under complex arithmetic, exponential or trigonometric operations. When losslessness cannot be guaranteed, the evaluation is deferred to the convertion to a non-literal scalar data type. These operations will be printed as-is if requested, without lossy evaluation. This is the only exception to the usually eager evaluation approach of S++.

- The implementation must only have bitwise operators, `+`, `-` and `*` being considered lossless literal operators. The conversion of the result of all others operators to a non-literal scalar must be considered `lossy`, and then force the user to use an explicit `lossy` conversion. Exceptions must not apply even if the result of the operation actually happens to be representable without loss for these operations. This case is assumed to be non-trivial and may cause significant discrepancies among implementations if would be left to their discretion: lossy behavior must be assumed anyway.
- Note that the lossy conversion of complex operations over scalar literals to a non-literal scalar data type is not guaranteed to be exact. It is left to the implementation, even if obviously it would be ideal to be able to compute each bit with absolute precision (and an extra one to round the LSB to nearest). Implementations based on floating-point math functions are heavily encouraged to perform computations on the most precise float available on the host. Hosts with fixed-point support only are encouraged to use at least 32 bits of fraction and 32 bits of integer, and ideally 64 bits of both if wide enough registers are available and that carry propagation is cheap enough. Applications are requested to assume that any implementation will provide a reasonable degree of precision and performance that the host can provide, that would make circumventing such a system unreasonable in the vast majority of use-cases. Regardless of machine-specific details, the application should expect the base implementation of S++ to detect remarkable operations on `transendental` and `real` scalars, such that they get converted to lossless equivalent operations that will likely result in more accuracy when evaluated by the platform-dependent implementation.

#### 1.4. String literals

S++ declares string literals similarly to C++, using double quotes. Example: `"Hello world!"`. This results into a compile-time array of the current character scalar. An encoding keyword can be prepended to the string literal to change its encoding from the current one.

The default encoding is UTF-8 and will produce arrays of `unsigned<8>` (or equivalently, `u8`). The encoding in the current module can be changed using the compile-time command `default_encoding([ENCODING_KEYWORD])`. The full list of available encodings is:
- `encoding_utf8`: UTF-8, will yield an array of `unsigned<8>` wrapped into a `StringUtf8` object. Access to this object yields `unsigned<32>` code points.
- `encoding_utf16`: UTF-16, will yield an array of `unsigned<16>` wrapped into a `StringUtf16` object. Access to this object yields `unsigned<16>` code points.
- `encoding_ucs2`: UCS-2, will yield an array of `unsigned<16>` wrapped into a `StringUtf16` object. Fixed-size characters, the code points are presented as-is in the array.
- `ascii`: ASCII encoding, will yield an array of `unsigned<8>` wrapped into a `StringAscii` object. Only the ASCII table can be encoded within the literal. Fixed-size characters, the code points are presented as-is in the array.

Note that none of these encodings present a null terminating character. The `toNullTerminated` method of these strings should be used to obtain a C (or similar)-compatible string.

### 2. Scope, assignment and evaluation

#### 2.1. Scopes

A scope defines a name space. A name space is a collection of unique identifiers which each resolve to an object. Non-declared identifiers resolve to the `undefined` value. Scopes can be nested one into each other by enclosing a sub-scope into `{}` braces. Within a sub-scope, identifiers in parent scopes can be resolved, however shadowing is not allowed (i.e. no object can be declared in a sub-scope which identifier is equivalent to the one of an object in any super scope).

A S++ module implicitly contains a top-level scope which gets enriched by the user when adding code to it. There is no distinction between a top-level scope and any sub-scope in its usage and properties. Scopes are objects like any other in the language and can be named using a variable assignment. Variables within a scope can be accessed using the `.` operator.

#### 2.2. Variables

A variable is a binding point that is defined within a particular scope, which identifier is unique within that scope. All undeclared variables resolve to the `undefined` value within a scope. `undefined` can be seen as the primitive value and type of every variable, until an assignment overrides it with a primary type which can get enriched with subsequent assignments of various types. A variable has then from zero (undeclared) to an indefinite amount of possible types. S++ statically evaluates the set of possible types for a variable throughout its entire lifetime, and at runtime allocates memory space only once at declaration for the storage for the largest type along with storing the type identifier.

Scalar values (along with their aggregation into objects) can generally be compile-time evaluated as well as runtime evaluated. Some classes of values such as scopes, class/struct/sequence definitions and functions cannot be runtime evaluated. In general, compile-time only values and compile-time and runtime values should not be mixed in their assignment to the same variable. However in practice, the only error that could arise is the runtime usage of a compile-time only value. As long as the application stays in compile-time domain, there is no restriction to the types a variable can be enriched with. A variable staying exclusively in compile-time domain typically acquires a single type at most that gets replaced at each assignment, as other previous types get pruned out by static analysis. In other words, compile-time programming in S++ is almost indistinguishable from typical dynamically-typed programming from a language usage standpoint.

Variable assignment happens using the `=` operator. The left token must be the variable identifier and the expression for its value must lie on the right.

```spp
// At top-level scope
// `a` would resolve to `undefined` if used here

topLevel = u32(0hBAAD_BEEF)

// Named scope called `app`
app = {
	// `314` binary unsigned fixed-point scalar literal, converted to a non-literal scalar with no loss
	a = dev_s(314)

	// `2.71` real with usage of a parent scope variable, yielding another real
	b = 2.71 + topLevel
}

// `a` would still resolve to `undefined` if used here
// However, `app.a` and `app.b` could be resolved to scalar values
```

As mentioned, variables can acquire multiple types:

```spp
// `a` is `undefined` at this point

a = true	// `a` is `bool` at this point

...		// Some runtime-dependent branch gets us to:
a = u32(314)	// `a` is `bool | u32` at this point

...		// Some other runtime-dependent branch gets us to:
a = void	// `a` is `bool | u32 | void` at this point
```

The `void` value is similar to `undefined` as they are both symbols which require no storage at runtime (aside from the type identifier), but deviates in its purpose and semantic.
- `undefined` can only be generated by language semantics from the absence of a binding to a variable, or more generally by the absence of user-defined objects in the usage of the language that could be expected to yield any sort of value. The user cannot assign a variable to `undefined`, as it would violate the requirement of `undefined` meaning zero user-defined type to a variable. However, the user can still generate `undefined` values in general (most likely for type checking, for example).
- `void` can only get user-generated. It is the conventional value to denote emptyness. An object of type `T` which may be or may not be defined could be expressed as a `T | void` type. The main practical difference with `undefined` is that the user explicitly assigns `void` to a variable, where `undefined` is the default value that cannot be assigned. The user is expected not to create other symbols that have the same practical usage as `void` and just use `void` instead.

#### 2.3. Evaluation

S++ is a early evaluated language. S++ attempts to evaluate at compile-time every expression, at long at it does not result in implicit data or semantic loss.

To achieve that, S++ marks every variable with either a compile-time value during parsing of statements, or a deferred unknown value under circumstances where compile-time evaluation is not reasonably possible. Examples of promotion to deferred evaluation include:
- Usage of a runtime-defined data, such as reading the keyboard in real-time or acquiring data from a file or the network.
	- File or network acquisition can be made compile-time using the appropriate compile-time functions such as `compile_file` (which works with a filepath or URL).
- Usage of not yet provided data in the compilation process: for example using an input variable of a function or template
	- If such variable happens to be compile-time evaluated, the target variable will be promoted from deferred back to compile-time evaluation.

When a function is invoked from the entry point of an application (at any scope depth), machine code is outputed only for variables for which assignment is still deferred, even after compile-time promotions.