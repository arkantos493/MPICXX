/**
 * @file include/mpicxx/detail/conversion.hpp
 * @author Marcel Breyer
 * @date 2020-05-31
 *
 * @brief Defines conversion functions used in the mpicxx library.
 */

#ifndef MPICXX_CONVERSION_HPP
#define MPICXX_CONVERSION_HPP

#include <string>

#include <mpicxx/detail/concepts.hpp>


namespace mpicxx::detail {

    /// @name conversion functions to std::string
    ///@{
    /**
     * @brief Converts a boolean value to its string representation.
     * @param[in] val the boolean value
     * @return `std::string("true")` or `std::string("false")`
     */
    inline std::string convert_to_string(const bool val) {
        using namespace std::string_literals;
        return val ? "true"s : "false"s;
    }
    /**
     * @brief Converts a single character to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
     * @param[in] c the character value
     * @return a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) with the same content as @p c
     */
    inline std::string convert_to_string(const char c) {
        return std::string(1, c);
    }
    /**
     * @brief Converts every *string* like type to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
     * @param[in] val a *string* like value
     * @return a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) with the same content as @p val
     */
    inline std::string convert_to_string(string auto&& val) {
        return std::string(std::forward<decltype(val)>(val));
    }
    /**
     * @brief Converts the given @p val to its string representation using the
     * [`to_string()](https://en.cppreference.com/w/cpp/string/basic_string/to_string) function.
     * @tparam T the convert to type
     * @param[in] val the value to convert
     * @return the string representation of @p val
     */
    template <typename T>
    inline std::string convert_to_string(T&& val) {
        using std::to_string;
        return to_string(std::forward<T>(val));
    }
    ///@}

}

#endif // MPICXX_CONVERSION_HPP
