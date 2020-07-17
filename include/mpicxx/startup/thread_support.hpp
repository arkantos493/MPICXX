/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-17
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Contains the level of thread support enum.
 * @details Additionally adds various functions to perform conversions from and to
 *          [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
 */

#ifndef MPICXX_THREAD_SUPPORT_HPP
#define MPICXX_THREAD_SUPPORT_HPP

#include <mpicxx/detail/conversion.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <mpi.h>

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string_view>

namespace mpicxx {

    /**
     * @brief Enum class for the different levels of thread support provided by MPI.
     * @details The values are monotonic: *single < funneled < serialized < multiple*.
     */
    enum class thread_support {
        /** only one thread will execute */
        single = MPI_THREAD_SINGLE,
        /** the process may be multi-threaded, but the application must ensure that only the main thread makes MPI calls (see @ref mpicxx::is_main_thread()) */
        funneled = MPI_THREAD_FUNNELED,
        /** the process may be multi-threaded, and multiple threads may make MPI calls, but only one at a time */
        serialized = MPI_THREAD_SERIALIZED,
        /** multiple threads may make MPI calls, with no restrictions */
        multiple = MPI_THREAD_MULTIPLE,
    };

    /// @name mpicxx::thread_support conversion functions
    ///@{
    /**
     * @brief Stream-insertion operator overload for the @ref mpicxx::thread_support enum class.
     * @param[inout] out an output stream
     * @param[in] ts the enum class value
     * @return the output stream
     */
    inline std::ostream& operator<<(std::ostream& out, const thread_support ts) {
        switch (ts) {
            case mpicxx::thread_support::single:
                out << "MPI_THREAD_SINGLE";
                break;
            case mpicxx::thread_support::funneled:
                out << "MPI_THREAD_FUNNELED";
                break;
            case mpicxx::thread_support::serialized:
                out << "MPI_THREAD_SERIALIZED";
                break;
            case mpicxx::thread_support::multiple:
                out << "MPI_THREAD_MULTIPLE";
                break;
        }
        return out;
    }
    /**
     * @brief Overload of the @ref mpicxx::to_string(T&&) function for the @ref mpicxx::thread_support enum class.
     * @param[in] ts the enum class value
     * @return the [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) representation of the enum class value
     * @nodiscard
     */
    [[nodiscard]]
    inline std::string to_string(const thread_support ts) {
        return fmt::format("{}", ts);
    }

    /**
     * @brief Converts the given string to the respective @ref mpicxx::thread_support value.
     * @details Expects the string value to be the MPI notation
     *          (e.g. [`"MPI_THREAD_SINGLE"`](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node303.htm) gets converted to
     *          @ref mpicxx::thread_support::single).
     * @param[in] sv the enum value represented as a string
     * @return the @ref mpicxx::thread_support representation of @p sv
     * @nodiscard
     *
     * @throws std::invalid_argument if the given value can't be converted to a @ref mpicxx::thread_support value
     */
    [[nodiscard]]
    inline thread_support thread_support_from_string(const std::string_view sv) {
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
     * @brief Stream-extraction operator overload for the @ref mpicxx::thread_support enum class.
     * @details Sets the [`std::ios::failbit`](https://en.cppreference.com/w/cpp/io/ios_base/iostate) if the given value can't be converted
     *          to a @ref mpicxx::thread_support value.
     * @param[inout] in an input stream
     * @param[out] ts the enum class
     * @return the input stream
     */
    inline std::istream& operator>>(std::istream& in, mpicxx::thread_support& ts) {
        try {
            std::string str;
            in >> str;
            ts = thread_support_from_string(str);
        } catch (const std::exception&) {
            in.setstate(std::ios::failbit);
        }
        return in;
    }
    ///@}

}

#endif // MPICXX_THREAD_SUPPORT_HPP