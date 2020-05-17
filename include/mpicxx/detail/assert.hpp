/**
 * @file include/mpicxx/detail/assert.hpp
 * @author Marcel Breyer
 * @date 2020-05-17
 *
 * @brief Provides more verbose assert alternatives, supporting MPI ranks.
 * @details The asserts are currently separated into three levels:
 * - **0**: **no** assertions are enabled (default)
 * - **1**: assertions that check preconditions, i.e. conditions that **must** hold for the function to complete successfully
 * - **2**: additionally to the assertions of level 1, assertions that check if the parameters make sense in the current context, but which
 * are **not required** for the function to complete successfully (e.g. increment of a past-the-end iterator), are added
 *
 * During cmake's configuration step, it is possible to enable a specific assertion level using the ´-DASSERTION_LEVEL´ option.
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
 * MPICXX_ASSERT_PRECONDITION(n >= 0, "Parameter can't be negative!: n = {}", n);
 * // alternative: MPICXX_ASSERT_SANITY
 *
 *
 * Precondition assertion 'n >= 0' failed on rank 1
 *   in file ./example.cpp
 *   in function 'int test(int)'
 *   @ line 42
 *
 * Parameter can't be negative!: n = -1
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
 * [*clang*](https://clang.llvm.org/)) is added if and only if the ASSERTION_LEVEL is greater than 0.
 * [*MSVC*](https://visualstudio.microsoft.com/de/vs/features/cplusplus/) currently doesn't print a stack trace at all.
 *
 * In addition the assertions call `MPI_Abort` if the assertion is executed within an active MPI environment,
 * [`std::abort`](https://en.cppreference.com/w/cpp/utility/program/abort) otherwise.
 */

#ifndef MPICXX_ASSERT_HPP
#define MPICXX_ASSERT_HPP

#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

#include <mpi.h>
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include <mpicxx/detail/source_location.hpp>

namespace mpicxx::detail {

    /**
     * @brief Enum class for the different assertion categories.
     */
    enum class assertion_category {
        /** precondition assertion */
        precondition,
        /** sanity assertion */
        sanity
    };
    /**
     * @brief Stream-insertion operator overload for the mpicxx::detail::assertion_category enum class.
     * @param[inout] out an output stream
     * @param[in] category the enum class value
     * @return the output stream
     */
    inline std::ostream& operator<<(std::ostream& out, const assertion_category category) {
        switch (category) {
            case assertion_category::precondition:
                out << "PRECONDITION";
                break;
            case assertion_category::sanity:
                out << "SANITY";
                break;
        }
        return out;
    }

    /**
     * @brief This function gets called by the `MPICXX_ASSERT_...` macros and does the actual assertion checking.
     * @details If the assert condition @p cond evaluates to ``false``, the condition, location, custom message and stack trace are printed
     * on the stderr steam. Afterwards the programs terminates with a call to MPI_Abort or
     * [`std::abort`](https://en.cppreference.com/w/cpp/utility/program/abort) respectively.
     *
     * @tparam Args parameter pack for the placeholder types
     * @param[in] cond the assert condition, aborts the program if evaluated to ``false``
     * @param[in] cond_str the assert condition as string for a better assert message
     * @param[in] category the mpicxx::detail::assertion_category
     * @param[in] loc the location where the assertion appeared
     * @param[in] msg the custom message printed after the assertion location (using the [**{fmt}**](https://github.com/fmtlib/fmt) syntax)
     * @param[in] args the arguments used to fill the [**{fmt}**](https://github.com/fmtlib/fmt) placeholders in the custom message
     *
     * @calls{
     * int MPI_Abort(MPI_Comm comm, int errorcode);     // at most once
     * }
     */
    template <typename... Args>
    inline void check(const bool cond, const char* cond_str, const assertion_category category, const source_location& loc,
            const char* msg, Args&&... args)
    {
        using namespace std::string_literals;
        // check if the assertion holds
        if (!cond) {
            fmt::memory_buffer buf;

            // format assertion message
            auto assertion_color = category == assertion_category::precondition ? fmt::fg(fmt::color::red) : fmt::fg(fmt::rgb(214, 136, 0));
            fmt::format_to(buf, "{} assertion {} failed {}\n",
                   fmt::format(assertion_color, "{}", category),
                   fmt::format(fmt::emphasis::bold | fmt::fg(fmt::color::green), "'{}'", cond_str),
                   (loc.rank().has_value() ? fmt::format("on rank {}", loc.rank().value()) : "without a running MPI environment"s));

            // format source location
            fmt::format_to(buf, "  in file {}\n  inf function {}\n  @ line {}\n\n", loc.file_name(), loc.function_name(), loc.line());

            // format custom assertion message
            fmt::format_to(buf, "{}\n\n", fmt::format(fmt::emphasis::bold | fmt::fg(fmt::color::red), msg, std::forward<Args>(args)...));

            // get stack trace
            fmt::format_to(buf, "{}", loc.stack_trace());

            // print full assertion message
            using std::to_string;
            fmt::print(stderr, to_string(buf));


            // call MPI_Abort only if we are currently in the MPI environment
            // i.e. MPI_Init has already been called, but MPI_Finalize not
            // TODO 2020-05-17 15:49 marcel: change to mpicxx::running()?
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


// ASSERTION_LEVEL
// 0 -> no assertions
// 1 -> only PRECONDITION assertions
// 2 -> PRECONDITION and SANITY assertions

/**
 * @def MPICXX_ASSERT_PRECONDITION
 * @brief Defines the @ref MPICXX_ASSERT_PRECONDITION macro if and only if the *ASSERTION_LEVEL*, as selected during cmake's configuration
 * step, is greater than 0.
 * @details This macro is responsible for all precondition checks. If a precondition of a function isn't met, the respective function isn't
 * guaranteed to finish successfully.
 *
 * An example is to check whether an iterator can be safely dereference or not.
 *
 * @param[in] cond the assert condition
 * @param[in] msg the custom assert message
 * @param[in] ... varying number of parameters to fill the [**{fmt}**](https://github.com/fmtlib/fmt) placeholders in
 * the custom assert message
 */
#if ASSERTION_LEVEL > 0
#define MPICXX_ASSERT_PRECONDITION(cond, msg, ...) \
        mpicxx::detail::check(cond, #cond, mpicxx::detail::assertion_category::precondition, mpicxx::detail::source_location::current(PRETTY_FUNC_NAME__), msg __VA_OPT__(,) __VA_ARGS__)
#else
#define MPICXX_ASSERT_PRECONDITION(cond, msg, ...)
#endif

/**
 * @def MPICXX_ASSERT_SANITY
 * @brief Defines the @ref MPICXX_ASSERT_SANITY macro if and only if the *ASSERTION_LEVEL*, as selected during cmake's configuration step,
 * is greater than 1.
 * @details This macro is responsible for all sanity checks. If a sanity check isn't successful, the respective function can still complete,
 * but the executed code wasn't necessarily meaningful.
 *
 * An example is the check whether an attempt is made to increment a past-the-end iterator.
 *
 * @param[in] cond the assert condition
 * @param[in] msg the custom assert message
 * @param[in] ... varying number of parameters to fill the [**{fmt}**](https://github.com/fmtlib/fmt) placeholders in
 * the custom assert message
 */
#if ASSERTION_LEVEL > 1
#define MPICXX_ASSERT_SANITY(cond, msg, ...) \
        mpicxx::detail::check(cond, #cond, mpicxx::detail::assertion_category::sanity, mpicxx::detail::source_location::current(PRETTY_FUNC_NAME__), msg __VA_OPT__(,) __VA_ARGS__)
#else
#define MPICXX_ASSERT_SANITY(cond, msg, ...)
#endif

#endif // MPICXX_ASSERT_HPP