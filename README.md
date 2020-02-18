# mpicxx - A header only C++ wrapper library for MPI

![Run tests](https://github.com/arkantos493/MPICXX/workflows/Run%20tests/badge.svg)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/9088a6289f864f19ba5869e103925b30)](https://www.codacy.com/manual/arkantos493/MPICXX?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=arkantos493/MPICXX&amp;utm_campaign=Badge_Grade)
[![Documentation](https://codedocs.xyz/arkantos493/MPICXX.svg)](https://codedocs.xyz/arkantos493/MPICXX/)

This library provides a small C++ wrapper for MPI libraries (like OpenMPI or MPICH).

## Getting Started

### Prerequisites

- a C++ compiler supporting C++20 concepts (e.g. [GCC](https://gcc.gnu.org/) trunk (10.0.0))
- [OpenMPI](https://www.open-mpi.org/) or [MPICH](https://www.mpich.org/) supporting the MPI 3 standard
- cmake (minimum required 3.5)
- [Doxygen](http://www.doxygen.nl/) (for documentation only)
- [fmt formatting library](https://github.com/fmtlib/fmt) (until `std::format` is supported)

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

- `-DENABLE_TESTS=ON/OFF`: uses the googletest framework (automatically installed if this option is set to `ON`) to enable the target `test` (default: `OFF`)

- `-DENABLE_DEATH_TEST=ON/OFF`: enables googletests death tests (currently not supported for MPI during its usage of fork()); only used if `ENABLE_TESTS` is set to `ON` (default: `OFF`)

- `-DENABLE_COVERALLS_REPORT=ON/OFF`: enables coverage reports via coveralls; only used if `ENABLE_TESTS` is set to `ON`; requires the build type to be set to `Debug` (default: `OFF`)

- `-DGENERATE_DOCUMENTATION=ON/OFF`: enables the target `doc` documentation; requires Doxygen (default: `OFF`)

- `-DASSERTION_LEVEL=0/1/2`: sets the assertion level; emits a warning if used in `Release` mode (default: `0`)
  - `0`: no assertions are active
  - `1`: only precondition assertions are active
  - `2`: additional sanity checks are activated

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
find_package(fmt REQUIRED) # until std::format is supported

# link it against your target
target_link_libraries(target mpicxx::mpicxx)
```

## Examples
For usage examples see the [Doxygen documentation](https://codedocs.xyz/arkantos493/MPICXX/) or the [Wiki](https://github.com/arkantos493/MPICXX/wiki).

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
