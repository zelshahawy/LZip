# Welcome to lzip

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


# Description

A minimalistic LZW (Lempel–Ziv–Welch) compression and decompression tool implemented in CPP and CMake as a header only project.
Supports dictionary growth up to 15-bit codes (max code = 32767) and a “stop-growing dictionary” strategy once the dictionary is full.


## Features

- **Encoding (compression)**:
  - Reads from a file or `stdin`.
  - Produces an `.lzw` file or writes to `stdout` (depending on environment variables).
  - Uses a variable code size, starting at 9 bits, up to a maximum of 15 bits.
  - Stops adding new dictionary entries once `nextCode` hits 32767, but continues encoding with existing entries.

- **Decoding (decompression)**:
  - Can reads from any file, should be a `.lzw` file, or `stdin`.
  - Produces a decompressed output file (`output.out`) or writes to `stdout` based on `CLI` Env variable.
  - Mirrors the encoder’s logic (9-bit initial code size, grows to 15 bits).
  - Ignores any leftover bits that cannot form a valid code (to avoid out-of-range dictionary references).

- **Threading and Parallelism**:
  - Supports encoding / decoding multiple files at times using threads up to a maximum of 8.
  - For example, you can call `./encode <filename[1]> <filename[2]> ... <filename[8]>`.
  - Has shown promising results when processing multiple, considerably big sized files.


# Prerequisites

Building lzip requires the following software installed:

* A C++17-compliant compiler
* CMake `>= 3.9`
* Doxygen (optional, documentation building is skipped if missing)

# Building lzip

The following sequence of commands builds lzip.
It assumes that your current working directory is the top-level directory
of the freshly cloned repository:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

The build process can be customized with the following CMake variables,
which can be set by adding `-D<var>={ON, OFF}` to the `cmake` call:

* `BUILD_TESTING`: Enable building of the test suite (default: `ON`)
* `BUILD_DOCS`: Enable building the documentation (default: `ON`)



# Testing lzip

When built according to the above explanation (with `-DBUILD_TESTING=ON`),
the C++ test suite of `lzip` can be run using
`ctest` from the build directory:

```
cd build
ctest
```


# Documentation

lzip provides a Sphinx-based documentation, that can
be browsed [online at readthedocs.org](https://lzip.readthedocs.io).
To build it locally, first ensure the requirements are installed by running this command from the top-level source directory:

```
pip install -r doc/requirements.txt
```

Then build the sphinx documentation from the top-level build directory:

```
cmake --build . --target sphinx-doc
```

The web documentation can then be browsed by opening `doc/sphinx/index.html` in your browser.
