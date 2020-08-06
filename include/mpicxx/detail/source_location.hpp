/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-21
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Provides a class similar to [`std::source_location`](https://en.cppreference.com/w/cpp/utility/source_location).
 * @details Differences are:
 *          - The new macro `MPICXX_PRETTY_FUNC_NAME__` is defined as `__PRETTY_FUNC__` ([GCC](https://gcc.gnu.org/) and
 *            [clang](https://clang.llvm.org/)), `__FUNCSIG__` ([MSVC](https://visualstudio.microsoft.com/de/vs/features/cplusplus/)) or
 *            `__func__` (otherwise). This macro can be used as first parameter to the static
 *            @ref mpicxx::detail::source_location::current() function to get a better function name.
 *          - Includes a member-function @ref mpicxx::detail::source_location::rank() which holds the current MPI rank (if a MPI environment
 *            is currently active).
 *          - The @ref mpicxx::detail::source_location::stack_trace() function can be used to print/get the current function call stack.
 */

#ifndef MPICXX_SOURCE_LOCATION_HPP
#define MPICXX_SOURCE_LOCATION_HPP

#include <fmt/format.h>
#include <mpi.h>

#include <optional>
#include <string>
#include <vector>

/**
 * @def MPICXX_PRETTY_FUNC_NAME__
 * @brief The @ref MPICXX_PRETTY_FUNC_NAME__ macro is defined as `__PRETTY_FUNC__` ([GCC](https://gcc.gnu.org/) and
 *        [clang](https://clang.llvm.org/)), `__FUNCSIG__` ([MSVC](https://visualstudio.microsoft.com/de/vs/features/cplusplus/)) or
 *        `__func__` (otherwise).
 * @details It can be used as compiler independent way to enable a better function name when used as first parameter to
 *          @ref mpicxx::detail::source_location::current().
 */
#ifdef __GNUG__
#include <execinfo.h>
#include <cxxabi.h>
#define MPICXX_PRETTY_FUNC_NAME__ __PRETTY_FUNCTION__
#elif _MSC_VER
#define MPICXX_PRETTY_FUNC_NAME__ __FUNCSIG__
#else
#define MPICXX_PRETTY_FUNC_NAME__ __func__
#endif

namespace mpicxx::detail {
    
    /**
     * @brief Represents information of a specific source code location.
     * @details Example usage:
     *          @snippet examples/detail/source_location.cpp mwe
     */
    class source_location {
    public:
        /**
         * @brief Constructs a new @ref mpicxx::detail::source_location with the respective information about the current call side.
         * @details The MPI rank is set to [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt) if an error occurred
         *          during the call to [*MPI_Comm_rank*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node155.htm)
         *          (an exception is thrown or a return code different than
         *          [*MPI_SUCCESS*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node222.htm) is returned).
         * @param[in] func the function name (including its signature if supported via the macro `MPICXX_PRETTY_FUNC_NAME__`)
         * @param[in] file the file name (absolute path)
         * @param[in] line the line number
         * @param[in] column the column number
         * @return the @ref mpicxx::detail::source_location holding the call side location information
         * @nodiscard
         *
         * @attention @p column is always (independent of the call side position) default initialized to 0!
         *
         * @calls{
         * int MPI_Initialized(int *flag);                 // exactly once
         * int MPI_Finalized(int *flag);                   // exactly once
         * int MPI_Comm_rank(MPI_Comm comm, int *rank);    // at most once
         * }
         */
        [[nodiscard]]
        static source_location current(
                const char* func = __builtin_FUNCTION(),
                const char* file = __builtin_FILE(),
                const int line = __builtin_LINE(),
                const int column = 0
                ) noexcept {
            source_location loc;
            loc.file_ = file;
            loc.func_ = func;
            loc.line_ = line;
            loc.column_ = column;
            try {
                // get the current MPI rank iff the MPI environment is active
                int is_initialized, is_finalized;
                MPI_Initialized(&is_initialized);
                MPI_Finalized(&is_finalized);
                if (static_cast<bool>(is_initialized) && !static_cast<bool>(is_finalized)) {
                    int rank;
                    int err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
                    if (err == MPI_SUCCESS) {
                        loc.rank_ = std::make_optional(rank);
                    }
                }
            } catch (...) {
                // something went wrong during the MPI calls -> no information could be retrieved
                loc.rank_ = std::nullopt;
            }
            return loc;
        }

        /**
         * @brief Returns the current stack trace.
         * @details For a better stack trace (precise function names) the linker flag `-rdynamic` is set if and only if **any**
         *          `MPICXX_ASSERTION_...` has been activated during [`Cmake`](https://cmake.org/)'s configuration step.
         *
         *          A sample stack trace (while `-rdynamic` is set) could look like:
         * @code
         * stack trace:
         *   #5    ./output.s: test(int) [+0x3]
         *   #4    ./output.s: foo() [+0x1]
         *   #3    ./output.s: main() [+0x1]
         *   #2    /lib/x86_64-linux-gnu/libc.so.6: __libc_start_main() [+0xe]
         *   #1    ./output.s: _start() [+0x2]
         * @endcode
         * @param[in] max_call_stack_size the maximum depth of the stack trace report
         * @return the stack trace
         * @nodiscard
         *
         * @attention The stack trace report is only available under [GCC](https://gcc.gnu.org/) and [clang](https://clang.llvm.org/)
         *            (to be precise: only if `__GNUG__` is defined). This function does nothing if `__GNUG__` isn't defined.
         */
        [[nodiscard]]
        static inline std::string stack_trace([[maybe_unused]] const int max_call_stack_size = 64) {
#if defined(MPICXX_ENABLE_STACK_TRACE) && defined(__GNUG__)
            using std::to_string;
            fmt::memory_buffer buf;
            fmt::format_to(buf, "stack trace:\n");

            std::vector<std::string> symbols;
            symbols.reserve(max_call_stack_size);

            {
                // storage array for stack trace address data
                std::vector<void*> addrlist(max_call_stack_size);
                // retrieve current stack addresses
                const int addrlen = backtrace(addrlist.data(), max_call_stack_size);

                // no stack addresses could be retrieved
                if (addrlen == 0) {
                    return fmt::format("{}    <empty, possibly corrupt>\n", to_string(buf));
                }

                // resolve addresses into symbol strings
                char** symbollist = backtrace_symbols(addrlist.data(), addrlen);
                for (int i = 0; i < addrlen; ++i) {
                    symbols.emplace_back(symbollist[i]);
                }
                free(symbollist);
            }

            // iterate over the returned symbol lines -> skip the first and second symbol because they are unimportant
            for (std::size_t i = 2; i < symbols.size(); ++i) {
                fmt::format_to(buf, "  #{:<6}", symbols.size() - i);

                // file_name(function_name+offset) -> split the symbol line accordingly
                const std::size_t position1 = std::min(symbols[i].find_first_of("("), symbols[i].size());
                const std::size_t position2 = std::min(symbols[i].find_first_of("+"), symbols[i].size());
                const std::size_t position3 = std::min(symbols[i].find_first_of(")"), symbols[i].size());

                std::string_view symbols_view(symbols[i]);
                std::string_view file_name = symbols_view.substr(0, position1);
                std::string function_name = symbols[i].substr(position1 + 1, position2 - position1 - 1);
                std::string_view function_offset = symbols_view.substr(position2, position3 - position2 - 1);

                // check if something went wrong while splitting the symbol line
                if (!file_name.empty() && !function_name.empty() && !function_offset.empty()) {
                    // demangle function name
                    int status = 0;
                    char* function_name_demangled = abi::__cxa_demangle(function_name.data(), nullptr, nullptr, &status);

                    if (status == 0) {
                        // demangling successful -> print pretty function name
                        fmt::format_to(buf, "{}: {} [{}]\n", file_name, function_name_demangled, function_offset);
                    } else {
                        // demangling failed -> print un-demangled function name
                        fmt::format_to(buf, "{}: {}() [{}]\n", file_name, function_name, function_offset);
                    }
                    free(function_name_demangled);
                } else {
                    // print complete symbol line if the splitting went wrong
                    fmt::format_to(buf, "{}\n", symbols[i]);
                }
            }
            return fmt::format("{}\n", to_string(buf));
#elif defined(MPICXX_ENABLED_STACK_TRACE) && !defined(__GNUG__)
// stack traces enabled but not supported
            return std::string("No stack trace supported!");
#else
// stack traces not supported
            return std::string{};
#endif
        }

        /**
         * @brief Returns the absolute path name of the file.
         * @return the file name
         * @nodiscard
         */
        [[nodiscard]]
        constexpr const char* file_name() const noexcept { return file_; }
        /**
         * @brief Returns the function name without additional signature information (i.e. return type or parameters).
         * @return the function name
         * @nodiscard
         */
        [[nodiscard]]
        constexpr const char* function_name() const noexcept { return func_; }
        /**
         * @brief Returns the line number.
         * @return the line number
         * @nodiscard
         */
        [[nodiscard]]
        constexpr int line() const noexcept { return line_; }
        /**
         * @brief Returns the column number.
         * @return the column number
         * @nodiscard
         *
         * @attention Default value in @ref mpicxx::detail::source_location::current() always 0!
         */
        [[nodiscard]]
        constexpr int column() const noexcept { return column_; }
        /**
         * @brief Returns the rank if a MPI environment is currently active.
         * @details If no MPI environment is currently active, the returned
         *          [`std::nullopt`](https://en.cppreference.com/w/cpp/utility/optional/nullopt) is empty.
         * @return a [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) containing the current MPI rank
         * @nodiscard
         */
        [[nodiscard]]
        constexpr std::optional<int> rank() const noexcept { return rank_; }

    private:
        const char* file_ = "unknown";
        const char* func_ = "unknown";
        int line_ = 0;
        int column_ = 0;
        std::optional<int> rank_ = std::nullopt;
    };

}

#endif // MPICXX_SOURCE_LOCATION_HPP