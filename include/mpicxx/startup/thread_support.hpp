/**
 * @file include/mpicxx/startup/thread_support.hpp
 * @author Marcel Breyer
 * @date 2020-02-20
 *
 * @brief Contains the level of thread support enum.
 * @details Additionally specializes a [*fmt*](https://github.com/fmtlib/fmt) formatter to print the actual thread support names.
 */

#ifndef MPICXX_THREAD_SUPPORT_HPP
#define MPICXX_THREAD_SUPPORT_HPP

#include <string_view>

#include <mpi.h>
#include <fmt/format.h>


namespace mpicxx {

    /**
     * @brief Enum class for the different levels of thread support provided by MPI.
     */
    enum thread_support {
        /** only one thread will execute */
        SINGLE = MPI_THREAD_SINGLE,
        /** the process may be multi-threaded, but the application must ensure that only the main thread makes MPI calls */
        FUNNELED = MPI_THREAD_FUNNELED,
        /** the process may be multi-threaded, and multiple threads may make MPI calls, but only one at a time */
        SERIALIZED = MPI_THREAD_SERIALIZED,
        /** multiple threads may call MPI, with no restrictions */
        MULTIPLE = MPI_THREAD_MULTIPLE,
    };

}

// TODO 2020-02-20 21:18 marcel: change from fmt::format to std::format
/**
 * @brief Custom [*fmt*](https://github.com/fmtlib/fmt) formatter for the mpicxx::thread_support enum class.
 */
template <>
struct fmt::formatter<mpicxx::thread_support> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const mpicxx::thread_support ts, FormatContext& ctx) {
        string_view name;
        switch (ts) {
            case mpicxx::thread_support::SINGLE:
                name = "MPI_THREAD_SINGLE";
                break;
            case mpicxx::thread_support::FUNNELED:
                name = "MPI_THREAD_FUNNELED";
                break;
            case mpicxx::thread_support::SERIALIZED:
                name = "MPI_THREAD_SERIALIZED";
                break;
            case mpicxx::thread_support::MULTIPLE:
                name = "MPI_THREAD_MULTIPLE";
                break;
        }
        return fmt::formatter<string_view>::format(name, ctx);
    }
};

#endif // MPICXX_THREAD_SUPPORT_HPP