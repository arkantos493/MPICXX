/**
 * @file assert.hpp
 * @author Marcel Breyer
 * @date 2019-11-24
 *
 * @brief Provides a more verbose assert alternative.
 *
 *
 * Old assert syntax and example output:
 * @code
 * assert(("Parameter can't be negative!", n > 0));
 *
 * output.s: ./example.cpp:42: void test(int): Assertion `("Parameter can't be negative!", n > 0)' failed.
 * @endcode
 *
 * New assert syntax and example output:
 * @code
 * MPICXX_ASSERT(n > 0, "Parameter can't be negative! : n = %i", n)
 *
 * Assertion 'n > 0' failed
 *     in file ./example.cpp
 *     in function 'test'
 *     @ line 42
 * Parameter can't be negative! : n = -1
 * @endcode
 * The new assert provides a ``printf`` like syntax for easier formatting of the error message.
 * The assertions are only enabled if ``NDEBUG`` is **not** set, i.e. only in debug mode.
 *
 * In addition this @ref MPICXX_ASSERT calls ``MPI_Abort`` if the assertion is called within a MPI environment.
 */

#ifndef MPICXX_ASSERT_HPP
#define MPICXX_ASSERT_HPP

#include <cstdio>
#include <iostream>

#include <mpi.h>

#include <mpicxx/utility/source_location.hpp>


namespace mpicxx::utility {
    /**
     * @brief This function gets called by the @ref MPICXX_ASSERT macro and does the actual assertion checking.
     *
     * If the assert condition @p cond evaluates to ``false``, the condition, location and custom message are printed on
     * ``std::cerr`` and afterwards halts the program.
     *
     * @code
     * Assertion 'n > 0' failed
     *     in file ./example.cpp
     *     in function 'test'
     *     @ line 42
     * Parameter can't be negative! : n = -1
     * @endcode
     *
     * @tparam Args parameter pack for the placeholder types
     * @param cond the assert condition, halts the program if evaluated to ``false``
     * @param cond_str the assert condition as string for a better assert message
     * @param loc the location where the assertion appeared
     * @param msg the custom message printed after the assertion location
     * @param args the arguments used to fill the ``printf`` like placeholders in the custom message
     */
    template <typename... Args>
    inline void check(const bool cond, const char* cond_str, const source_location& loc, const char* msg, const Args... args) {
        if (!cond) {
            std::cerr << "Assertion '" << cond_str << "' failed\n"
                      << "    in file " << loc.file_name() << "\n"
                      << "    in function '" << loc.function_name() << "'\n"
                      << "    @ line " << loc.line() << std::endl;
            fprintf(stderr, msg, args...);
            std::cerr << std::endl;
            // call MPI_Abort only if we are currently in the MPI environment
            // i.e. MPI_Init has already been called, but MPI_Finalize not
            int is_initialized = 0;
            MPI_Initialized(&is_initialized);
            int is_finalized = 0;
            MPI_Finalized(&is_finalized);
            if (is_initialized != 0 && is_finalized == 0) {
                // we are currently in an active MPI environment
                // -> call MPI_Abort
                MPI_Abort(MPI_COMM_WORLD, 1);
            } else {
                // we are currently NOT in an active MPI environment
                // -> call normal std::abort()
                std::abort();
            }
        }
    }
}

/**
 * @def MPICXX_ASSERT()
 * Defines the @ref MPICXX_ASSERT macro iff ``NDEBUG`` is **not** set, otherwise this macro does nothing.
 *
 * This macro effectively calls the @ref check() function by insertion the condition as string argument, adding the current
 * location information (by using @ref mpicxx::utility::source_location) and forwarding all other parameters.
 *
 * @param cond the assert condition
 * @param msg the custom assert message
 * @param ... varying number of parameters to fill the ``printf`` like placeholders in the custom assert message
 */
#ifdef NDEBUG
#define MPICXX_ASSERT(cond, msg, ...)
#else
#define MPICXX_ASSERT(cond, msg, ...) mpicxx::utility::check(cond, #cond, mpicxx::utility::source_location::current(), msg, ##__VA_ARGS__)
#endif

#endif // MPICXX_ASSERT_HPP
