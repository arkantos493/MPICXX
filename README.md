# mpicxx - A header only C++ wrapper library for MPI

[![Build Status](https://travis-ci.org/arkantos493/MPICXX.svg?branch=master)](https://travis-ci.org/arkantos493/MPICXX)
[![Coverage Status](https://coveralls.io/repos/github/arkantos493/MPICXX/badge.svg?branch=master)](https://coveralls.io/github/arkantos493/MPICXX?branch=master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9088a6289f864f19ba5869e103925b30)](https://www.codacy.com/manual/arkantos493/MPICXX?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=arkantos493/MPICXX&amp;utm_campaign=Badge_Grade)
[![Documentation](https://codedocs.xyz/arkantos493/MPICXX.svg)](https://codedocs.xyz/arkantos493/MPICXX/)

This library provides a small C++ wrapper for MPI libraries (like OpenMPI or MPICH).

## Getting Started

### Prerequisites

- GCC trunk (10.0.0)
- [OpenMPI](https://www.open-mpi.org/) or [MPICH](https://www.mpich.org/) supporting the MPI 3 standard
- cmake (minimum required 3.5)
- Doxygen (for documentation only)
- [fmt formatting library](https://github.com/fmtlib/fmt)

### Installing

This library supports the normal cmake tool chain.
```bash
git clone git@github.com:arkantos493/MPICXX.git
cd MPICXX
mkdir build && cd build
cmake [options] ..
make -j $(nproc)
```
Supported options are: TODO
- `-DCMAKE_BUILD_TYPE=Debug/Release/...`
- `CMAKE_INSTALL_PREFIX=...`: set the installation path
- `-DENABLE_TESTS=ON/OFF`: uses the googletest framework (which gets automatically installed if this option is turned on) to enable the target `test`
- `-DENABLE_COVERALLS_REPORT=ON/OFF`: enables coverage reports via coveralls, only used if `ENABLE_TESTS` is set to `ON`; requires the build type set to `Debug`
- `-DGENERATE_DOCUMENTATION=ON/OFF`: enables the target `doc` documentation, requires Doxygen, Doxyrest (gets automatically installed if this option is turned on (so users don't have to set paths in `doc/doxyrest-config-lua` and `doc/conf.py`)) and Sphinx

## Running the tests

After a successful `make` (with a previously `cmake` call with option `-DENABLE_TESTS=ON`) simply run:
```bash
ctest
```

## Deployment

To install the library simple use:
```bash
make install
```
or
```bash
sudo make install
```
To use this library simple add the following lines to your `CMakeLists.txt` file:
```cmake
# find the library
find_package(mpicxx CONFIG REQUIRED)

# link it against your target
target_link_libraries(target mpicxx::mpicxx)
```

## Examples
Coming later...

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
