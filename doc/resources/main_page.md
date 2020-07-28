# mpicxx - A header only C++ wrapper library for MPI

This library provides a small C++ wrapper for MPI libraries (like OpenMPI or MPICH).

## Getting Started

### Prerequisites

- [GCC 10.1](https://gcc.gnu.org/gcc-10/) or higher; clang isn't supported until it supports the `<concepts>` header
- [OpenMPI](https://www.open-mpi.org/) or [MPICH](https://www.mpich.org/) supporting the MPI v3.1 standard
- [CMake](https://cmake.org/) (minimum required 3.14.4)
- [{fmt}](https://github.com/fmtlib/fmt) formatting library
- [doxygen](http://www.doxygen.nl/) (for documentation only, version 1.8.11 **required**)

### Installing

This library supports the normal cmake toolchain.
```bash
git clone git@github.com:arkantos493/MPICXX.git
cd MPICXX
mkdir build && cd build
cmake [options] ..
make -j
```

### Supported CMake options (incomplete for options without MPICXX prefix)

| option                                       | default value   | description |
| -------------------------------------------- | :-------------: | ----------- |
| `CMAKE_BUILD_TYPE`                           | `Release` | specifies the build type on single-configuration generators |
| `CMAKE_INSTALL_PREFIX`                       | `/usr/local/include` | install directory used by `make install` |
| `MPICXX_ENABLE_TESTS`                        | `Off` | use the [googletest](https://github.com/google/googletest) framework (automatically installed if this option is set to `On`) to enable the `make test` target |
| `MPICXX_ENABLE_DEATH_TESTS`                  | `Off` | enables gtest's death tests (currently not supported for MPI during its internal usage of `fork()`); only used if `MPICXX_ENABLE_TESTS` is set to `On` |
| `MPICXX_GENERATE_DOCUMENTATION`              | `Off` | enables the documentation target `make doc`; requires doxygen |
| `MPICXX_GENERATE_TEST_DOCUMENTATION`         | `Off` | additionally document test cases; only used if `MPICXX_GENERATE_DOCUMENTATION` is set to `On` |
| `MPICXX_ASSERTION_LEVEL`                     | `0`   | sets the assertion level; emits a warning if used in `Release` mode; <ul><li>`0` = no assertions</li><li>`1` = only precondition assertions</li><li>`2` = precondition and sanity assertions</li></ul> |
| `MPICXX_ENABLE_STACK_TRACE`                  | `On`  | enable stack traces for the source location implementation |
| `DMPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS` | `32`  | sets the maximum number of `atfinalize` callback functions |

## Running the tests

This library supports `ctest` together with `cmake`:
```bash
cmake -DMPICXX_ENABLE_TESTS=On ..
make -j
ctest
```

## Generate the documentation

This library integrates doxygen within `cmake`:
```bash
cmake -DMPICXX_GENERATE_DOCUMENTATION=On ..
make doc
```

To additionally generate documentation for the test cases:
```bash
cmake -DMPICXX_GENERATE_DOCUMENTATION=On -DMPICXX_GENERATE_TEST_DOCUMENTATION=On ..
make doc
```

## Deployment

This library supports the `make install` target:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make install
```

For a debug build one should enable assertions:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DMPICXX_ASSERTION_LEVEL=2 ..
make install
```

To then use this library simple add the following lines to your `CMakeLists.txt` file:
```cmake
# find the library
find_package(mpicxx CONFIG REQUIRED)
find_package(fmt REQUIRED)

# link it against your target
target_link_libraries(target PUBLIC mpicxx::mpicxx)
```

## Examples
For usage examples see the [doxygen documentation](https://arkantos493.github.io/MPICXX/) or the [wiki](https://github.com/arkantos493/MPICXX/wiki).

## License

This project is licensed under the MIT License - see the <a href="https://github.com/arkantos493/MPICXX/blob/master/LICENSE.md">LICENSE.md</a> file for details.
