cmake_minimum_required(VERSION 3.5)

project("mpicxx" 
        VERSION "0.1.0" 
        LANGUAGES CXX
        DESCRIPTION "A header only C++ wrapper library for MPI"
        HOMEPAGE_URL "https://github.com/arkantos493/MPICXX")

# configure config file
set(CONFIG_IN ${CMAKE_CURRENT_SOURCE_DIR}/include/mpicxx/config.hpp.in)
set(CONFIG_OUT ${CMAKE_CURRENT_SOURCE_DIR}/include/mpicxx/config.hpp)
configure_file(${CONFIG_IN} ${CONFIG_OUT} @ONLY)

# enable/disable options
include(CMakeDependentOption)
option(ENABLE_TESTS "Generate tests" OFF)
cmake_dependent_option(ENABLE_COVERALLS_REPORT "Generate coveralls data" OFF 
                       "ENABLE_TESTS" OFF)
cmake_dependent_option(ENABLE_DEATH_TESTS "Enables gtest's death tests (not support with MPI)" OFF
                       "ENABLE_TESTS" OFF)
option(GENERATE_DOCUMENTATION "Generate documentation" OFF)


# set assertion category
set(possible_assertion_categories PRECONDITION SANITY NONE ALL)
set(ASSERTION_CATEGORY NONE CACHE STRING "The active assertion categories.")
set_property(CACHE ASSERTION_CATEGORY PROPERTY STRINGS ${possible_assertion_categories})
if (ENABLE_DEATH_TESTS)
    message(WARNING "Enabled all assertion categories because death tests are enabled.")
    set(ASSERTION_CATEGORY ALL)
endif ()
set(active_assertion_categories 0)
set(length 0)
# assertion_category holds all provided assertion categories
set(assertion_category)
list(APPEND assertion_category ${ASSERTION_CATEGORY})
list(LENGTH assertion_category length)

# if ALL or NONE is specified they have to be the only categories!
if (NONE IN_LIST assertion_category AND length GREATER 1)
    message(FATAL_ERROR "If NONE is present it must be the only option!")
endif()
if (ALL IN_LIST assertion_category AND length GREATER 1)
    message(FATAL_ERROR "If ALL is present it must be the only option!")
endif()

# iterate over all provided categories
foreach(category ${ASSERTION_CATEGORY})
    # the category must be one of the predefined
    if (NOT ${category} IN_LIST possible_assertion_categories)
        message(FATAL_ERROR "ASSERTION_CATEGORY must only include ${possible_assertion_categories} but includes ${category}")
    else ()
        message(STATUS "Enabled assertion category: ${category}")
        # interpret "active_assertion_categories" as bit set and set the respective bit
        # ALL          -> set all bits
        # NONE         -> set no bit
        # PRECONDITION -> set 1st bit
        # SANITY       -> set 2nd bit
        if (${category} STREQUAL ALL)
            list(LENGTH possible_assertion_categories length)
            math(EXPR length "${length} - 3")
            if (length GREATER_EQUAL 0)
                foreach (i RANGE ${length})
                    math(EXPR active_assertion_categories "${active_assertion_categories} + (1 << ${i})")
                endforeach ()
            endif ()
        elseif (${category} STREQUAL NONE)
            set(active_assertion_categories 0)
        else()
            list(FIND possible_assertion_categories ${category} idx)
            math(EXPR active_assertion_categories "${active_assertion_categories} + (1 << ${idx})")
        endif()
    endif ()
endforeach()

# add custom cmake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


# set build type to Release if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type (Debug/Release)" FORCE)
endif (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

# to generate coveralls reports be sure that the current build type is debug
if (ENABLE_COVERALLS_REPORT AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(FATAL_ERROR "In order to use coveralls reports please change the build type to debug")
endif (ENABLE_COVERALLS_REPORT AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")


### based on http://dominikberner.ch/cmake-interface-lib/ ###

# create header-only (interface) library
add_library(${PROJECT_NAME} INTERFACE)
target_compile_definitions(${PROJECT_NAME} INTERFACE ASSERTION_CATEGORIES=${active_assertion_categories})

# find MPI and add it to the library target
find_package(MPI REQUIRED)
target_include_directories(${PROJECT_NAME} INTERFACE ${MPI_CXX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} INTERFACE MPI::MPI_CXX)


# set include directories
target_include_directories(
    ${PROJECT_NAME} INTERFACE 
        $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>    # include path if library is used directly by another cmake target
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)           # include path if project is installed

# set needed compiler dependencies
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# set important compiler flags
add_compile_options(-Wall -Wextra -pedantic)
if (NOT (active_assertion_categories EQUAL 0))
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using Clang or GCC
        message(STATUS "Enabled '-rdynamic' linker flag for better stack traces in assertions")
        add_link_options(-rdynamic)
    endif()
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
        PATTERN "config.hpp.in" EXCLUDE)
#install documentation if generated
if (GENERATE_DOCUMENTATION)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/doc/html DESTINATION share/doc)
endif (GENERATE_DOCUMENTATION)


# tests for interface library
if (ENABLE_TESTS)
    message(STATUS "Enabled tests using google's test framework")
    # install test framework in third_party directory
    include(GoogleTest)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif (ENABLE_TESTS)

if (ENABLE_DEATH_TESTS)
    message(STATUS "Enabled death tests using gtest's ASSERT_DEATH()")
endif(ENABLE_DEATH_TESTS)

# enable coverage reports using coveralls
if (ENABLE_COVERALLS_REPORT)
    ### based on https://github.com/JoakimSoderberg/coveralls-cmake ###
    message(STATUS "Enabled coverage reports using coveralls")
    include(Coveralls)
    coveralls_turn_on_coverage()
    # TODO: test if coverage reports work
    file(GLOB_RECURSE COVERAGE_SRCS "${PROJECT_SOURCE_DIR}/include/mpicxx(*.hpp)")
    coveralls_setup("${COVERAGE_SRCS}" ON
                    "${PROJECT_SOURCE_DIR}/cmake/modules/")
endif(ENABLE_COVERALLS_REPORT)


# generate documentation if requested
if (GENERATE_DOCUMENTATION)
    message(STATUS "Enabled documentation generation with Doxygen")
    add_subdirectory(doc)
endif (GENERATE_DOCUMENTATION)


