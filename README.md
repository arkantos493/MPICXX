# mpicxx - A header only C++ wrapper library for MPI

![Test with GCC](https://github.com/arkantos493/MPICXX/workflows/Test%20with%20GCC/badge.svg)
![Test with clang](https://github.com/arkantos493/MPICXX/workflows/Test%20with%20clang/badge.svg)

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9088a6289f864f19ba5869e103925b30)](https://www.codacy.com/manual/arkantos493/MPICXX?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=arkantos493/MPICXX&amp;utm_campaign=Badge_Grade)
[![Generic badge](https://img.shields.io/badge/code-documented-<COLOR>.svg)](https://arkantos493.github.io/MPICXX/)

This library provides a small C++ wrapper for MPI libraries (like OpenMPI or MPICH).

## Getting Started

### Prerequisites

- at least [GCC 10.1](https://gcc.gnu.org/gcc-10/)
- [OpenMPI](https://www.open-mpi.org/) or [MPICH](https://www.mpich.org/) supporting the MPI 3 standard
- [CMake](https://cmake.org/) (minimum required 3.13)
- [{fmt}](https://github.com/fmtlib/fmt) formatting library
- [Doxygen](http://www.doxygen.nl/) (for documentation only, version 1.8.11 **required**)

### Installing

This library supports the normal cmake tool chain.
```bash
git clone git@github.com:arkantos493/MPICXX.git
cd MPICXX
mkdir build && cd build
cmake [options] ..
make -j $(nproc)
```
Supported configuration options are:
- `-DCMAKE_BUILD_TYPE=Debug/Release/...` (default: `Release`)

- `-DCMAKE_INSTALL_PREFIX=...`: set the installation path (default: `/usr/local/include`)

- `-DMPICXX_ENABLE_TESTS=ON/OFF`: uses the googletest framework (automatically installed if this option is set to `ON`) to enable the target `test` (default: `OFF`)

- `-DMPICXX_ENABLE_DEATH_TESTS=ON/OFF`: enables googletests death tests (currently not supported for MPI during its usage of fork()); only used if `MPICXX_ENABLE_TESTS` is set to `ON` (default: `OFF`)

- `-DMPICXX_GENERATE_DOCUMENTATION=ON/OFF`: enables the target `doc` documentation; requires Doxygen (default: `OFF`)

- `-DMPICXX_ASSERTION_LEVEL=0/1/2`: sets the assertion level; emits a warning if used in `Release` mode (default: `0`)
  - `0`: no assertions are active
  - `1`: only precondition assertions are active
  - `2`: additional sanity checks are activated
  
- `-DMPICXX_ENABLE_STACK_TRACE=ON/OFF`: enable stack traces for the source location implementation (default: `ON`)
  
- `-DMPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS=0...N`: sets the maximum number of atfinalize callback functions (default: `32`)

## Running the tests

After a successful `make` (with a previously `cmake` call with option `-DMPICXX_ENABLE_TESTS=ON`) simply run:
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
find_package(fmt REQUIRED)

# link it against your target
target_link_libraries(target mpicxx::mpicxx)
```

## Examples
For usage examples see the [Doxygen documentation](https://arkantos493.github.io/MPICXX/) or the [Wiki](https://github.com/arkantos493/MPICXX/wiki).

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
