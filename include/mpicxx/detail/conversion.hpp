/**
 * @file include/mpicxx/detail/conversion.hpp
 * @author Marcel Breyer
 * @date 2020-03-19
 *
 * @brief Defines conversion functions used in the mpicxx library.
 */

#ifndef MPICXX_CONVERSION_HPP
#define MPICXX_CONVERSION_HPP

#include <string>

#include <mpicxx/detail/concepts.hpp>


namespace mpicxx::detail {

    std::string convert_to_string(const bool val) {
        using namespace std::string_literals;
        return val ? "true"s : "false"s;
    }
    std::string convert_to_string(string auto&& val) {
        return std::string(std::forward<decltype(val)>(val));
    }
    template <typename T>
    std::string convert_to_string(T&& val) {
        using std::to_string;
        return to_string(std::forward<T>(val));
    }

}

#endif // MPICXX_CONVERSION_HPP
