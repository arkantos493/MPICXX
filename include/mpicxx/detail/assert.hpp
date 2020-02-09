/**
 * @file include/mpicxx/detail/assert.hpp
 * @author Marcel Breyer
 * @date 2020-02-09
 *
 * @brief Provides more verbose assert alternatives, supporting MPI ranks.
 * @details The asserts are currently separated into four categories:
 * - *NONE*: **no** assertions are enabled at all (default)
 * - *ALL*: enables **all** assertions
 * - *PRECONDITION*: assertions that check preconditions, i.e. conditions that **must** hold for the function to complete successfully
 * - *SANITY*: assertions that check if the parameters make sense in the current context, but which are **not required** for the function to
 * complete successfully (e.g. increment of a past-the-end iterator)
 *
 * During cmake's configuration step, it is possible to disable/enable only a subset of these assertions
 * (via the `-DASSERTION_CATEGORY` option).
 *
 * Old assertion syntax and example output:
 * @code
 * assert(("Parameter can't be negative!", n > 0));
 *
 *
 * output.s: ./example.cpp:42: void test(int): Assertion `("Parameter can't be negative!", n > 0)' failed.
 * @endcode
 *
 * New assertion syntax and example output:
 * @code
 * MPICXX_ASSERT_PRECONDITION(n > 0, "Parameter can't be negative! : n = %i", n);
 * // alternatives: MPICXX_ASSERT_SANITY
 *
 *
 * Precondition assertion 'n > 0' failed on rank 1
 *   in file ./example.cpp
 *   in function 'int test(int)'
 *   @ line 42
 *
 * Parameter can't be negative! : n = -1
 *
 * stack trace:
 *   #7    ./output.s: test(int) [+0x3]
 *   #6    ./output.s: foo() [+0x1]
 *   #5    ./output.s: bar() [+0x]
 *   #4    ./output.s: baz() [+0x]
 *   #3    ./output.s: main() [+0x1]
 *   #2    /lib/x86_64-linux-gnu/libc.so.6: __libc_start_main() [+0xe]
 *   #1    ./output.s: _start() [+0x2]
 * @endcode
 * For a meaningful stack trace to be printed the linker flag `-rdynamic` ([*GCC*](https://gcc.gnu.org/) and
 * [*clang*](https://clang.llvm.org/)) is added if and only if **any** assertion category is used during cmake's configuration.
 * [*MSVC*](https://visualstudio.microsoft.com/de/vs/features/cplusplus/) currently doesn't print a stack trace at all.
 *
 * In addition the assertions call ``MPI_Abort`` if the assertion is executed within an active MPI environment.
 */

#ifndef MPICXX_ASSERT_HPP
#define MPICXX_ASSERT_HPP

#include <cstdio>
#include <iostream>
#include <sstream>

#include <mpi.h>

#include <mpicxx/detail/source_location.hpp>

namespace mpicxx::detail {
    /**
     * @brief This function gets called by the `MPICXX_ASSERT_...` macros and does the actual assertion checking.
     * @details If the assert condition @p cond evaluates to ``false``, the condition, location, custom message and stack trace are printed
     * on the stderr steam. Afterwards the programs terminates with a call to MPI_Abort or
     * [`std::abort`](https://en.cppreference.com/w/cpp/utility/program/abort) respectively.
     *
     * @tparam Args parameter pack for the placeholder types
     * @param cond the assert condition, halts the program if evaluated to ``false``
     * @param cond_str the assert condition as string for a better assert message
     * @param assertion_category the assertion category (one of: *PRECONDITION*, *SANITY*)
     * @param loc the location where the assertion appeared
     * @param msg the custom message printed after the assertion location
     * @param args the arguments used to fill the ``printf`` like placeholders in the custom message
     */
    template <typename... Args>
    inline void check(const bool cond, const char* cond_str, const char* assertion_category, const source_location& loc,
            const char* msg, const Args... args) {
        // check if the assertion holds
        if (!cond) {
            std::stringstream ss;
            ss << assertion_category << " assertion '" << cond_str << "' failed";
            // if we are in an active MPI environment, print current rank
            if (loc.rank().has_value()) {
                ss << " on rank " << loc.rank().value();
            }
            ss << "\n"
               << "  in file " << loc.file_name() << "\n"
               << "  in function '" << loc.function_name() << "'\n"
               << "  @ line " << loc.line() << "\n\n";

            // TODO 2020-02-07 22:24 marcel: change to std::format as soon as possible
            int size = std::snprintf(nullptr, 0, msg, args...);
            std::string buffer(size, ' ');
            std::snprintf(buffer.data(), size + 1, msg, args...);
            ss << buffer << "\n\n";

            // write stacktrace into the string stream
            loc.stack_trace(ss);

            // print whole message at once to prevent race conditions
            std::cerr << ss.str();

            // call MPI_Abort only if we are currently in the MPI environment
            // i.e. MPI_Init has already been called, but MPI_Finalize not
            if (loc.rank().has_value()) {
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

// TODO 2020-02-05 21:33 marcel: remove later
/**
 * @def MPICXX_ASSERT()
 * Defines the @ref MPICXX_ASSERT macro iff ``NDEBUG`` is **not** set, otherwise this macro does nothing.
 *
 * This macro effectively calls the @ref check() function by insertion the condition as string argument, adding the current
 * location information (by using @ref mpicxx::detail::source_location) and forwarding all other parameters.
 *
 * @param cond the assert condition
 * @param msg the custom assert message
 * @param ... varying number of parameters to fill the ``printf`` like placeholders in the custom assert message
 */
#ifdef NDEBUG
#define MPICXX_ASSERT(cond, msg, ...)
#else
#define MPICXX_ASSERT(cond, msg, ...) mpicxx::detail::check(cond, #cond, "General", mpicxx::detail::source_location::current(PRETTY_FUNC_NAME__), msg, ##__VA_ARGS__)
#endif


#define CHECK_BIT(var,pos) ( (var >> pos) & 1 )

// ASSERTION_CATEGORIES interpreted as bit set
// bit at position 0 set -> enable PRECONDITION assertion macro
// bit at position 1 set -> enable SANITY assertion macro

/**
 * @def MPICXX_ASSERT_PRECONDITION
 * @brief Defines the @ref MPICXX_ASSERT_PRECONDITION macro if and only if it was explicitly selected (or implicitly via *ALL*) during
 * cmake's configuration step, otherwise this macro expands to `nop`.
 * @details This macro is responsible for all precondition checks. If a precondition of a function isn't met, the respective function isn't
 * guaranteed to finish successfully.
 *
 * An example is to check whether an iterator can be safely dereference or not.
 *
 * @param cond the assert condition
 * @param msg the custom assert message
 * @param ... varying number of parameters to fill the ``printf`` like placeholders in the custom assert message
 */
#if CHECK_BIT(ASSERTION_CATEGORIES, 0) == 1
#define MPICXX_ASSERT_PRECONDITION(cond, msg, ...) \
        mpicxx::detail::check(cond, #cond, "Precondition", mpicxx::detail::source_location::current(PRETTY_FUNC_NAME__), msg, ##__VA_ARGS__)
#else
#define MPICXX_ASSERT_PRECONDITION(cond, msg, ...)
#endif

/**
 * @def MPICXX_ASSERT_SANITY
 * @brief Defines the @ref MPICXX_ASSERT_SANITY macro if and only if it was explicitly selected (or implicitly via *ALL*) during
 * cmake's configuration step, otherwise this macro expands to `nop`.
 * @details This macro is responsible for all sanity checks. If a sanity check isn't successful, the respective function can still complete,
 * but the executed code wasn't necessarily meaningful.
 *
 * An example is the check whether an attempt is made to increment a past-the-end iterator.
 *
 * @param cond the assert condition
 * @param msg the custom assert message
 * @param ... varying number of parameters to fill the ``printf`` like placeholders in the custom assert message
 */
#if CHECK_BIT(ASSERTION_CATEGORIES, 1) == 1
#define MPICXX_ASSERT_SANITY(cond, msg, ...) \
        mpicxx::detail::check(cond, #cond, "Sanity", mpicxx::detail::source_location::current(PRETTY_FUNC_NAME__), msg, ##__VA_ARGS__)
#else
#define MPICXX_ASSERT_SANITY(cond, msg, ...)
#endif

#endif // MPICXX_ASSERT_HPP