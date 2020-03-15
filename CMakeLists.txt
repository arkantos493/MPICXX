cmake_minimum_required(VERSION 3.13)

project("mpicxx" 
        VERSION "0.3.1"
        LANGUAGES CXX
        DESCRIPTION "A header only C++ wrapper library for MPI"
        HOMEPAGE_URL "https://github.com/arkantos493/MPICXX")

# configure version file
set(CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/include/mpicxx/version/version.hpp.in)
set(CONFIG_OUT ${CMAKE_CURRENT_SOURCE_DIR}/include/mpicxx/version/version.hpp)
configure_file(${CONFIG_IN} ${CONFIG_OUT} @ONLY)

# configure .codedocs file
set(CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/.codedocs.in)
set(CONFIG_OUT ${CMAKE_CURRENT_SOURCE_DIR}/.codedocs)
configure_file(${CONFIG_IN} ${CONFIG_OUT} @ONLY)

# enable/disable options
include(CMakeDependentOption)
option(ENABLE_TESTS "Generate tests" OFF)
cmake_dependent_option(ENABLE_DEATH_TESTS "Enables gtest's death tests (not support with MPI)" OFF
                       "ENABLE_TESTS" OFF)
option(GENERATE_DOCUMENTATION "Generate documentation" OFF)


# set maximum possible number of atfinalize callback functions
set(MAX_NUMBER_OF_ATFINALIZE_CALLBACKS 32 CACHE STRING "The maximum possible number of atfinalize callback functions.")
if (NOT (${MAX_NUMBER_OF_ATFINALIZE_CALLBACKS} MATCHES "^[0-9]+$"))
    message(FATAL_ERROR "MAX_NUMBER_OF_ATFINALIZE_CALLBACKS must be a positive integer")
endif ()
message(STATUS "MAX_NUMBER_OF_ATFINALIZE_CALLBACKS: ${MAX_NUMBER_OF_ATFINALIZE_CALLBACKS}")


# set assertion level
set(max_assertion_level 2)
set(ASSERTION_LEVEL 0 CACHE STRING "The active assertion level.")

# check if assertion level is valid
if (NOT (${ASSERTION_LEVEL} MATCHES "^[0-9]+$" AND ${ASSERTION_LEVEL} GREATER_EQUAL 0 AND ${ASSERTION_LEVEL} LESS_EQUAL ${max_assertion_level}))
    message(FATAL_ERROR "ASSERTION_LEVEL must be an integer in the range: 0-${max_assertion_level}")
endif ()
message(STATUS "ASSERTION_LEVEL: ${ASSERTION_LEVEL}")

if (ENABLE_DEATH_TESTS AND ${ASSERTION_LEVEL} LESS ${max_assertion_level})
    message(WARNING "Enabled highest assertion level because death tests are enabled.")
    set(ASSERTION_LEVEL ${max_assertion_level})
endif ()


# add custom cmake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


# set build type to Release if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type (Debug/Release)" FORCE)
endif ()


# warn if assertions are used in a Release build
if (CMAKE_BUILD_TYPE MATCHES "Release" AND ${ASSERTION_LEVEL} GREATER 0)
    message(WARNING "Build type Release with active assertions: this may negatively impact performance!")
endif ()

### based on http://dominikberner.ch/cmake-interface-lib/ ###

# create header-only (interface) library
add_library(${PROJECT_NAME} INTERFACE)
target_compile_definitions(${PROJECT_NAME} INTERFACE ASSERTION_LEVEL=${ASSERTION_LEVEL})
target_compile_definitions(${PROJECT_NAME} INTERFACE MAX_NUMBER_OF_ATFINALIZE_CALLBACKS=${MAX_NUMBER_OF_ATFINALIZE_CALLBACKS})

# find MPI and add it to the library target
find_package(MPI REQUIRED)
target_include_directories(${PROJECT_NAME} INTERFACE ${MPI_CXX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} INTERFACE MPI::MPI_CXX)

# fmt
find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE fmt::fmt)

# set include directories
target_include_directories(
    ${PROJECT_NAME} INTERFACE 
        $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>    # include path if library is used directly by another cmake target
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)           # include path if project is installed

# set needed compiler dependencies
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# set general compiler flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # clang and GCC specific flags
    set(WARNING_FLAGS -Wall -Wextra -Wpedantic -Wextra-semi -Wnull-dereference -Wswitch-enum)

    # clang specific flags
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND WARNING_FLAGS -Wno-gnu-zero-variadic-macro-arguments)
    endif ()

    # GCC specific flags
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND WARNING_FLAGS -Wduplicated-cond -Wduplicated-branches -Wsuggest-override)
    endif ()

    message(STATUS "Enabled '${WARNING_FLAGS}' compiler flags to catch more potential errors")
    target_compile_options(${PROJECT_NAME} INTERFACE ${WARNING_FLAGS})
endif ()

# set compiler linker flag for better stack traces
if (NOT (active_assertion_categories EQUAL 0))
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using Clang or GCC
        message(STATUS "Enabled '-rdynamic' linker flag for better stack traces in assertions")
        target_link_options(${PROJECT_NAME} INTERFACE -rdynamic)
    endif ()
endif ()

# set targets to install
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}     # all files that are neither executables, shared libraries or header files
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}     # all shared libraries files
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})    # all executables

# manage version comparison
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

# generate configuration file
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# create and copy install-targets file
install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# create file containing the build configuration and version information
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)


# install headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/mpicxx DESTINATION include
        PATTERN "version.hpp.in" EXCLUDE)
#install documentation if generated
if (GENERATE_DOCUMENTATION)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/doc/html DESTINATION share/doc)
endif ()


# tests for interface library
if (ENABLE_TESTS)
    message(STATUS "Enabled tests using google's test framework")
    # install test framework in third_party directory
    include(GoogleTest)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif ()

if (ENABLE_DEATH_TESTS)
    message(STATUS "Enabled death tests using gtest's ASSERT_DEATH()")
endif ()


# generate documentation if requested
if (GENERATE_DOCUMENTATION)
    message(STATUS "Enabled documentation generation with Doxygen")
    add_subdirectory(doc)
endif ()


