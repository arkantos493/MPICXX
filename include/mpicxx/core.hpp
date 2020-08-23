/**
 * @file include/mpicxx/core.hpp
 * @author Marcel Breyer
 * @date 2020-08-23
 *
 * @brief Core header which includes every other necessary header file, i.e. \c \#include` <mpicxx/core.hpp>` is sufficient to use every
 *        function or class of the mpicxx library.
 */

#ifndef MPICXX_CORE_HPP
#define MPICXX_CORE_HPP

// include all necessary headers TODO 2020-02-20 21:58 marcel: add other headers
// chrono
#include <mpicxx/chrono/clock.hpp>
// info
#include <mpicxx/info/info.hpp>
#include <mpicxx/info/runtime_info.hpp>
// startup
#include <mpicxx/startup/mpicxx_main.hpp>
#include <mpicxx/startup/multiple_spawner.hpp>
#include <mpicxx/startup/single_spawner.hpp>
// version
#include <mpicxx/version/version.hpp>


/// The main namespace of this library. Nearly all functions are located in this namespace.
namespace mpicxx {}

/// This namespace contains all constants and functions to query information of the current library and MPI (library) versions.
namespace mpicxx::version {}

/// This namespace is for implementation details and **should not** be used directly be users.
namespace mpicxx::detail {}

/// This namespace is for implementation details used to treat enums or enum classes as bitmasks.
namespace mpicxx::detail::bitmask {}

/// This namespace contains ADL (argument dependent lookup) helpers.
namespace mpicxx::detail::adl_helper {}

#endif // MPICXX_CORE_HPP