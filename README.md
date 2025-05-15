# Welcome to lzip

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/zelshahawy/LZip/ci.yml?branch=main)](https://github.com/zelshahawy/LZip/actions/workflows/ci.yml)
[![Documentation Status](https://readthedocs.org/projects/LZip/badge/)](https://LZip.readthedocs.io/)
[![codecov](https://codecov.io/gh/zelshahawy/LZip/branch/main/graph/badge.svg)](https://codecov.io/gh/zelshahawy/LZip)

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
