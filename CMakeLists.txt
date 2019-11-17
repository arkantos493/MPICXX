cmake_minimum_required(VERSION 3.5)

project("mpicxx" 
        VERSION "0.1.0" 
        LANGUAGES CXX
        DESCRIPTION "A header only C++ wrapper library for MPI"
        HOMEPAGE_URL "https://github.com/arkantos493/MPICXX")

# set important compiler flags
add_compile_options(-Wall -Wextra -pedantic -fconcepts)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# enable/disable generation of documentation
option(GEN_DOC "Generate documentation" OFF)
option(COVERALLS "Generate coveralls data" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# set build type to Release if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as no build type was specified")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type (Debug/Release)" FORCE)
endif (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)


# create header-only (interface) library
add_library(${PROJECT_NAME} INTERFACE)

# find MPI and add it to the library target
find_package(MPI REQUIRED)
include_directories(${PROJECT_NAME} ${MPI_CXX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} INTERFACE MPI::MPI_CXX)


# set include directories
target_include_directories(
    ${PROJECT_NAME} INTERFACE 
        $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>    # include path if library is used directly by another cmake target
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)           # include path if project is installed

# set needed compiler dependencies
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

# set targets to install
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}     # all files that are neither executables, shared libraries or header files
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}     # all shared libraries files
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})    # all executables
                                                        # INCLUDE ommited because INSTALL_INTERFACE already specifies this

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
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/mpicxx DESTINATION include)


# test interface library
include(${PROJECT_SOURCE_DIR}/cmake/GoogleTest.cmake)
include(CTest)
enable_testing()
add_subdirectory(test)

if (COVERALLS)
    include(Coveralls)
    coveralls_turn_on_coverage()
    file(GLOB_RECURSE COVERAGE_SRCS "${PROJECT_SOURCE_DIR}/inlcude/mpicxx(*.hpp")
    # Create the coveralls target.
    coveralls_setup(
        "${COVERAGE_SRCS}" # The source files.
        ON                 # If we should upload.
        "${PROJECT_SOURCE_DIR}/cmake/modules/") # (Optional) Alternate project cmake module path.
endif()

# generate documentation if requested
if (GEN_DOC)
    add_subdirectory(doc)
endif (GEN_DOC)


