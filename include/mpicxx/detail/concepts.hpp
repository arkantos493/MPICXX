/**
 * @file include/mpicxx/detail/concepts.hpp
 * @author Marcel Breyer
 * @date 2020-02-08
 *
 * @brief Defines concepts used in the mpicxx library.
 */

#ifndef MPICXX_CONCEPTS_HPP
#define MPICXX_CONCEPTS_HPP

#include <type_traits>

namespace mpicxx::detail {

    /**
     * @brief Concept that describes every *string* like type, i.e. [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string),
     * [`std::string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view), `const char*` and `char[]`.
     * @tparam T the compared to type
     */
    template <typename T>
    concept string = std::is_constructible_v<std::string_view, T>;

}

#endif // MPICXX_CONCEPTS_HPP
