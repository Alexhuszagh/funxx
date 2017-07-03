PyCPP
=====

Python-like C++ environment with independent, lightweight utilities for cross-platform C++ development.

[![Build Status](https://travis-ci.org/Alexhuszagh/funxx.svg?branch=master)](https://travis-ci.org/Alexhuszagh/funxx)
[![Build Status](https://tea-ci.org/api/badges/Alexhuszagh/pycpp/status.svg)](https://tea-ci.org/Alexhuszagh/pycpp)
[![Build status](https://ci.appveyor.com/api/projects/status/b8fh8431rcu3wc3q?svg=true)](https://ci.appveyor.com/project/Alexhuszagh/pycpp)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/Alexhuszagh/pycpp/blob/master/LICENSE.md)

**Table of Contents**

- [Introduction](#introduction)
- [Design](#design)
- [Building](#building)
- [Platforms](#platforms)
- [Contributors](#contributors)
- [License](#license)

## Introduction

Modern C++ development depends on a mix of archaic C-libraries with poorly documented interfaces, and template-driven, header-only libraries that produce significant binary and compile time bloat. PyCPP aims to be a collection loosely interconnected features missing from the C++ standard library, boasting lean headers, fast compile times, and permissive licenses, to facilitate app development with minimal overhead.

PyCPP is, in many ways, a spiritual port of Python back to C++. There are idiomatic ways to do certain tasks, and there is a right and a wrong way. "Zero-cost abstractions" don't exist: enabling infinite abstraction leads to unmaintainable complexity. PyCPP also ports Python's [hashlib](https://docs.python.org/3/library/hashlib.html), [os.path](https://docs.python.org/2/library/os.path.html#module-os.path), and [string](https://docs.python.org/2/library/string.html) modules, providing familiar interfaces

## Design

PyCPP is broken down into multiple core parts:

1. A macro-driven abstraction library to detect compiler features.
2. Code page conversion routines.
3. Filesystem utilities.
4. High-level features common in other standard libraries.

Combined, the library has less than 1500 (is this still accurate??) total lines of C++ headers, with binary sizes of < 1MB, and each module has only a few dependencies, making it easy to extract individual modules for use in another project.

**Abstraction Library**

The files [os.h](/src/os.h), [compiler.h](/src/compiler.h), [architecture.h](/src/architecture.h), [processor.h](/src/processor.h), and [byteorder.h](/src/byteorder.h) provide an abstraction platform to detect the current compiler, compiler version, operating system, system endianness, and processor. These are the largest dependencies, with ~1000 lines of code.

**Code Page Conversion**

Any modern application should be Unicode aware, and PyCPP includes routines for [code point conversions](/src/unicode.h) and [codec conversions](codec.h), as well as [Unicode-aware case conversions](/src/casemap.h). They provide two APIs: a modern wrapper for STL string conversions, and a C-API taking a src and dst buffer.

Supported features include:

- UTF-8, UTF-16, and UTF-32 conversions
- std::string, std::u16string, and std::u32string conversions.
- Lowercase, uppercase, capitalized, and titlecase Unicode conversions.

**Fileystem**

PyCPP includes a high-level filesystem library, a spiritual port of the `os.path` module from Python.

Supported features include:

- Portable stat and lstat
- Path normalization
- Environment variable expansion

**High-Level Features**

**_General_**

- [Thread-local storage](/src/tls.h).
- [Optional](/src/optional.h) type.
- [Generic "any"](/src/any.h) type.
- [Bit-flag aware scoped enums](/src/enum.h).
- [string_view](/str/view/string.h) and [vector_view](/src/view/vector.h)

**_Codec_**

- [Base16](/src/base16.h), [base32](/src/base32.h), and [Base64](/src/base64.h) encoding and decoding.
- [Hex](/src/hex.h) encoding and decoding.
- [Punycode](/src/punycode.h) encoding and decoding.
- [Url](/src/url.h) encoding and decoding.
- [International domain names](/src/idna.h) encoding and decoding.

**_Streams_**

- [Wide API streams](/src/stream/fstream.h) for Windows.
- [Filtering streambufs](/src/stream/filter.h).

**_Random_**

- [Pseudorandom](/src/random.h) number generators.
- [Cryptographic random](/src/random.h) number generators.

**_Cryptography_**

- [Secure memory](/src/allocator.h) allocation and deallocation.
- [Secure buffers](/src/string.h).
- [Cryptographic hash](/src/hashlib.h) functions.
- [Cryptographic ciphers](/src/cipher.h).

**_Iterables_**

- [Iterator adaptors](/src/iterator.h).
- [Combinations, permutation](/src/itertools/sampling.h) and [cartesian products](/src/itertools/product.h).

**_JSON_**

- [DOM](/src/json/dom.h) parser.
- [SAX](/src/json/sax.h) parser.

**_XML_**

- [DOM](/src/xml/dom.h) parser.
- [SAX](/src/xml/sax.h) parser.

**_Compression_**

**WARNING:** PyCPP includes cryptographic hashes and ciphers, which are tested via fuzzing. The buffer containing these objects is reset upon object destruction, to minimize side-channel attacks. However, they should be assumed to be insecure, until audited.

## Building

Simply clone, configure with CMake, and build.

```bash
git clone https://github.com/Alexhuszagh/pycpp.git
git submodule update --init --recursive
cmake .                         # `-DBUILD_TESTS=ON`
make -j 5                       # "msbuild pycpp.sln" for MSVC
```

## Platforms

PyCPP is continually built with the following compiler and compiler versions:

- Clang 3.8+
- GCC 5.3+
- MinGW 5.3.0 (MXE, MinGW, and MSYS2)
- Visual Studio 14 2015
- Visual Studio 15 2017

## Contributors

- Alex Huszagh

## License

MIT, see [license](LICENSE.md).
