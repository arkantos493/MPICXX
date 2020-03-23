/**
 * @file include/mpicxx/startup/thread_support.hpp
 * @author Marcel Breyer
 * @date 2020-03-23
 *
 * @brief Contains the level of thread support enum.
 * @details Additionally specializes a [*fmt*](https://github.com/fmtlib/fmt) formatter to print the actual thread support names.
 */

#ifndef MPICXX_THREAD_SUPPORT_HPP
#define MPICXX_THREAD_SUPPORT_HPP

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string_view>

#include <mpi.h>
#include <fmt/format.h>


namespace mpicxx {

    /**
     * @brief Enum class for the different levels of thread support provided by MPI.
     * @details The values are monotonic: single < funneled < serialized < multiple.
     */
    enum thread_support {
        /** only one thread will execute */
        single = MPI_THREAD_SINGLE,
        /** the process may be multi-threaded, but the application must ensure that only the main thread makes MPI calls (see @ref mpicxx::is_main_thread()) */
        funneled = MPI_THREAD_FUNNELED,
        /** the process may be multi-threaded, and multiple threads may make MPI calls, but only one at a time */
        serialized = MPI_THREAD_SERIALIZED,
        /** multiple threads may call MPI, with no restrictions */
        multiple = MPI_THREAD_MULTIPLE,
    };

    /// @name thread_support conversion functions
    ///@{
    // TODO 2020-03-18 15:29 marcel: change from fmt::format to std::format
    /**
     * @brief Stream-insertion operator overload for the mpicxx::thread_support enum class.
     * @param[inout] out an output stream
     * @param[in] ts the enum class value
     * @return the output stream
     */
    inline std::ostream& operator<<(std::ostream& out, const thread_support ts) {
        out << fmt::format("{}", ts);
        return out;
    }
    // TODO 2020-03-19 14:35 marcel: change from fmt::format to std::format
    /**
     * @brief `to_string` overload (using ADL) for the mpicxx::thread_support enum class.
     * @param[in] ts the enum class value
     * @return the converted [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string)
     */
    inline std::string to_string(const thread_support ts) {
        return fmt::format("{}", ts);
    }

    // TODO 2020-03-18 15:29 marcel: change from fmt::format to std::format
    /**
     * @brief Converts the given string to the respective mpicxx::thread_support value.
     * @details Excepts the string value to be the MPI notation (e.g. `"MPI_THREAD_SINGLE"` gets converted to
     * `mpicxx::thread_support::single`).
     * @param[in] sv the enum value represented as string
     * @return the converted enum value
     *
     * @throws std::invalid_argument if the given value can't be converted to a mpicxx::thread_support value
     */
    inline thread_support enum_from_string(const std::string_view sv) {
        if (sv.compare("MPI_THREAD_SINGLE") == 0) {
            return thread_support::single;
        } else if (sv.compare("MPI_THREAD_FUNNELED") == 0) {
            return thread_support::funneled;
        } else if (sv.compare("MPI_THREAD_SERIALIZED") == 0) {
            return thread_support::serialized;
        } else if (sv.compare("MPI_THREAD_MULTIPLE") == 0) {
            return thread_support::multiple;
        } else {
            throw std::invalid_argument(fmt::format("Can't convert \"{}\" to mpicxx::thread_support!", sv));
        }
    }

    /**
     * @brief Stream-extraction operator overload for the mpicxx::thread_support enum class.
     * @param[inout] in an input stream
     * @param[out] ts the enum class
     * @return the input stream
     *
     * @throws std::invalid_argument if the given value can't be converted to a mpicxx::thread_support value
     */
    inline std::istream& operator>>(std::istream& in, mpicxx::thread_support& ts) {
        std::string str;
        in >> str;
        ts = enum_from_string(str);
        return in;
    }
    ///@}

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
            case mpicxx::thread_support::single:
                name = "MPI_THREAD_SINGLE";
                break;
            case mpicxx::thread_support::funneled:
                name = "MPI_THREAD_FUNNELED";
                break;
            case mpicxx::thread_support::serialized:
                name = "MPI_THREAD_SERIALIZED";
                break;
            case mpicxx::thread_support::multiple:
                name = "MPI_THREAD_MULTIPLE";
                break;
        }
        return fmt::formatter<string_view>::format(name, ctx);
    }
};

#endif // MPICXX_THREAD_SUPPORT_HPP