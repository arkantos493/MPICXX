/**
 * @file include/mpicxx/detail/source_location.hpp
 * @author Marcel Breyer
 * @date 2020-05-17
 *
 * @brief Provides a `source_location` class similar to [`std::source_location`](https://en.cppreference.com/w/cpp/utility/source_location).
 * @details Differences are:
 * - The new macro `PRETTY_FUNC_NAME__` is defined as `__PRETTY_FUNC__` ([*GCC*](https://gcc.gnu.org/) and
 * [*clang*](https://clang.llvm.org/)), `__FUNCSIG__` ([*MSVC*](https://visualstudio.microsoft.com/de/vs/features/cplusplus/)) or
 * `__func__` (otherwise). This macro can be used as first parameter to the static @ref mpicxx::detail::source_location::current()
 * function to get a better function name.
 * - Includes a member named `rank` which holds the current MPI rank (if a MPI environment is currently active).
 * - The @ref mpicxx::detail::source_location::stack_trace() function can be used to print/get the current function call stack.
 */

#ifndef MPICXX_SOURCE_LOCATION_HPP
#define MPICXX_SOURCE_LOCATION_HPP

#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include <mpi.h>
#include <fmt/format.h>

/**
 * @def PRETTY_FUNC_NAME__
 * @brief The @ref PRETTY_FUNC_NAME__ macro is defined as `__PRETTY_FUNC__` ([*GCC*](https://gcc.gnu.org/) and
 * [*clang*](https://clang.llvm.org/)), `__FUNCSIG__` ([*MSVC*](https://visualstudio.microsoft.com/de/vs/features/cplusplus/)) or
 * `__func__` (otherwise).
 * @details It can be used as compiler independent way to enable a better function name when used as first parameter to
 * @ref mpicxx::detail::source_location::current().
 */
#ifdef __GNUG__
#include <execinfo.h>
#include <cxxabi.h>
#define PRETTY_FUNC_NAME__ __PRETTY_FUNCTION__
#elif _MSC_VER
#define PRETTY_FUNC_NAME__ __FUNCSIG__
#else
#define PRETTY_FUNC_NAME__ __func__
#endif

namespace mpicxx::detail {
    /**
     * @brief Represents information of a specific source code location.
     * @details Example usage:
     * @include examples/detail/source_location.cpp
     */
    class source_location {
    public:
        /**
         * @brief Constructs a new source_location with the respective information about the current call side.
         * @param[in] func the function name (including its signature if supported via the macro `PRETTY_FUNC_NAME__`)
         * @param[in] file the file name (absolute path)
         * @param[in] line the line number
         * @param[in] column the column number
         * @return the source_location holding the call side location information
         *
         * @attention @p column is always (independent of the call side position) default initialized to 0!
         *
         * @calls{
         * int MPI_Initialized(int *flag);                  // exactly once
         * int MPI_Finalized(int *flag);                    // exactly once
         * int MPI_Comm_rank(MPI_Comm comm, int *rank);     // at most once
         * }
         */
        static source_location current(
                const std::string_view func = __builtin_FUNCTION(),
                const std::string_view file = __builtin_FILE(),
                const int line = __builtin_LINE(),
                const int column = 0
                ) noexcept {
            source_location loc;
            loc.file_ = file;
            loc.func_ = func;
            loc.line_ = line;
            loc.column_ = column;
            // TODO 2020-05-17 15:49 marcel: change to mpicxx::running()?
            try {
                // get the current MPI rank iff the MPI environment is active
                int is_initialized, is_finalized;
                MPI_Initialized(&is_initialized);
                MPI_Finalized(&is_finalized);
                if (static_cast<bool>(is_initialized) && !static_cast<bool>(is_finalized)) {
                    int rank;
                    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
                    loc.rank_ = std::optional<int>(rank);
                }
            } catch (...) {
                // something went wrong during the MPI calls -> no information could be retrieved
                loc.rank_ = std::nullopt;
            }
            return loc;
        }

        /**
         * @brief Prints the current stack trace to the given output stream @p out.
         * @details For a better stack trace (precise function names) the linker flag `-rdynamic` is set if and only if **any**
         * MPICXX_ASSERTION has been activated during cmake's configuration step.
         *
         * A sample output (while `-rdynamic` set) could look like:
         * @code
         *  stack trace:
         *   #5    ./output.s: test(int) [+0x3]
         *   #4    ./output.s: foo() [+0x1]
         *   #3    ./output.s: main() [+0x1]
         *   #2    /lib/x86_64-linux-gnu/libc.so.6: __libc_start_main() [+0xe]
         *   #1    ./output.s: _start() [+0x2]
         * @endcode
         * @param[in] max_call_stack_size the maximum depth of the stack trace report
         *
         * @attention The stack trace report is only available under [*GCC*](https://gcc.gnu.org/) and [*clang*](https://clang.llvm.org/)
         * (to be precise: only if `__GNUG__` is defined). This function does nothing if `__GNUG__` isn't defined.
         */
        static inline std::string stack_trace([[maybe_unused]] const int max_call_stack_size = 64) {
#if defined(ENABLE_STACK_TRACE) && defined(__GNUG__)
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
#elif defined(ENABLED_STACK_TRACE) && !defined(__GNUG__)
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
         */
        constexpr const std::string& file_name() const noexcept { return file_; }
        /**
         * @brief Returns the function name without additional signature information (i.e. return type or parameters).
         * @return the function name
         */
        constexpr const std::string& function_name() const noexcept { return func_; }
        /**
         * @brief Returns the line number.
         * @return the line number
         */
        constexpr int line() const noexcept { return line_; }
        /**
         * @brief Returns the column number.
         * @return the column number
         *
         * @attention Default value in @ref mpicxx::detail::source_location::current() always 0!
         */
        constexpr int column() const noexcept { return column_; }
        /**
         * @brief Returns the rank if a MPI environment is currently active.
         * @details If no MPI environment is currently active, the returned
         * [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) is empty.
         * @return a `std::optional<int>` containing the current rank
         */
        constexpr std::optional<int> rank() const noexcept { return rank_; }

    private:
        std::string file_ = "unknown";
        std::string func_ = "unknown";
        int line_ = 0;
        int column_ = 0;
        std::optional<int> rank_ = std::nullopt;
    };

}

#endif // MPICXX_SOURCE_LOCATION_HPP