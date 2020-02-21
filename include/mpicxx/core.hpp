/**
 * @file include/mpicxx/core.hpp
 * @author Marcel Breyer
 * @date 2020-02-21
 *
 * @brief Core header which includes every other necessary header file, i.e. \#`include <mpicxx/core.hpp>` is sufficient to use every
 * function or class of the mpicxx library.
 */

#ifndef MPICXX_CORE_HPP
#define MPICXX_CORE_HPP

// TODO 2020-02-20 21:58 marcel: add other headers
// include all necessary headers
#include <mpicxx/chrono/clock.hpp>
#include <mpicxx/info/info.hpp>
#include <mpicxx/version/version.hpp>



// namespace documentation

/// The main namespace of this library. Nearly all functions are located in this namespace.
namespace mpicxx {}

/// This namespace contains all constants and functions to query information of the current library and MPI (library) versions.
namespace mpicxx::version {}

/// This namespace is for implementation details and **should not** be used directly be users.
namespace mpicxx::detail {}

#endif // MPICXX_CORE_HPP
