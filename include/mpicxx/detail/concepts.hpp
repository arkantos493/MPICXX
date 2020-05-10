/**
 * @file include/mpicxx/detail/concepts.hpp
 * @author Marcel Breyer
 * @date 2020-05-10
 *
 * @brief Defines concepts used in the mpicxx library.
 */

#ifndef MPICXX_CONCEPTS_HPP
#define MPICXX_CONCEPTS_HPP

#include <string_view>
#include <type_traits>
#include <utility>


namespace mpicxx::detail {

    /// @name custom internally used C++20 concepts
    ///@{
    /**
     * @brief Concept that describes every *string* like type, i.e. [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string),
     * [`std::string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view), `const char*` and `char[]`.
     * @tparam T the compared to type
     */
    template <typename T>
    concept string = std::is_constructible_v<std::string_view, T>;

    /**
     * @brief Concept that describes a function that accepts any number of parameters (including none) and returns an `int`.
     * @details Such a function is `int main()`.
     * @tparam T a callable
     * @tparam Args optional parameters (potential empty)
     */
    template <typename T, typename... Args>
    concept main_pointer = std::is_invocable_r_v<int, T, Args...>;

    /**
     * @brief Concept that describes a function that does accept an `int`, an array of c-style strings and any number of parameters
     * (including none) and returns an `int`.
     * @details Such a function is `int main(int argc, char** argv)`.
     * @tparam T a callable
     * @tparam Args optional parameters (potential empty)
     */
    template <typename T, typename... Args>
    concept main_args_pointer = std::is_invocable_r_v<int, T, int, char**, Args...>;

    /**
     * @brief Concept that describes a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair).
     * @tparam T the compared to type
     */
    template <typename T>
    concept is_pair = requires (T p) {
        { p.first }  -> std::convertible_to<typename std::remove_cvref_t<T>::first_type>;
        { p.second } -> std::convertible_to<typename std::remove_cvref_t<T>::second_type>;
    };
    ///@}

}

#endif // MPICXX_CONCEPTS_HPP
