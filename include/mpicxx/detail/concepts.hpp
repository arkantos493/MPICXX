/**
 * @file include/mpicxx/detail/concepts.hpp
 * @author Marcel Breyer
 * @date 2020-02-20
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

    /**
     * @brief Concept that describes a function that doesn't accept any parameter and returns an `int`.
     * @details Such a function is `int main()`.
     * @tparam T a callable
     */
    template <typename T>
    concept main_pointer = std::is_invocable_r_v<int, T>;

    /**
     * @brief Concept that describes a function that does accept an `int` and an array of c-style strings and returns an `int`.
     * @details Such a function is `int main(int argc, char** argv)`.
     * @tparam T a callable
     */
    template <typename T>
    concept main_args_pointer = std::is_invocable_r_v<int, T, int, char**>;

}

#endif // MPICXX_CONCEPTS_HPP
