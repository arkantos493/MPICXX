/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-20
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines concepts used in the mpicxx library.
 */

#ifndef MPICXX_CONCEPTS_HPP
#define MPICXX_CONCEPTS_HPP

#include <string_view>
#include <type_traits>
#include <utility>

// forward declare mpicxx classes
namespace mpicxx {
    class single_spawner;
    class multiple_spawner;
    class info;
}

namespace mpicxx::detail {

    /// @name custom, internally used, C++20 concepts
    ///@{
    /**
     * @brief @concept{ @ref is_string<T> }
     *        Concept that describes every *string* like type, i.e. [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string),
     *        [`std::string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view), `const char*` and `char[]`.
     * @tparam T the compared to type
     */
    template <typename T>
    concept is_string = std::is_constructible_v<std::string, T>;
    /**
     * @brief @concept{ @ref is_c_string<T> }
     *        Concept that describes a c-style string, i.e. `const char*`, `char*`, `const char[]` and `char[]`.
     * @tparam T the compared to type
     */
    template <typename T>
    concept is_c_string = std::is_same_v<std::decay_t<T>, const char*> || std::is_same_v<std::decay_t<T>, char*>;

    /**
     * @brief @concept{ @ref is_main_pointer<T, Args...> }
     *        Concept that describes a function that accepts any number of parameters (including none) and returns an `int`.
     * @details Such a function is `int main()`.
     * @tparam T a callable
     * @tparam Args optional parameters (potentially empty)
     */
    template <typename T, typename... Args>
    concept is_main_pointer = std::is_invocable_r_v<int, T, Args...>;
    /**
     * @brief @concept{ @ref is_main_args_pointer<T, Args...> }
     *        Concept that describes a function that does accept an `int`, an array of c-style strings and any number of additional
     *        parameters (including none) and returns an `int`.
     * @details Such a function is `int main(int argc, char** argv)`.
     * @tparam T a callable
     * @tparam Args optional parameters (potentially empty)
     */
    template <typename T, typename... Args>
    concept is_main_args_pointer = std::is_invocable_r_v<int, T, int, char**, Args...>;

    /**
     * @brief @concept{ @ref is_pair<T> }
     *        Concept that describes a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) like type.
     * @tparam T the compared to type
     */
    template <typename T>
    concept is_pair = requires (T p) {
        { p.first }  -> std::convertible_to<typename std::remove_cvref_t<T>::first_type>;
        { p.second } -> std::convertible_to<typename std::remove_cvref_t<T>::second_type>;
    };

    /**
     * @brief @concept{ @ref is_spawner<T> }
     *        Concept that describes a spawner class, i.e. either @ref mpicxx::single_spawner or @ref mpicxx::multiple_spawner.
     * @tparam T the compared to type
     */
    template <typename T>
    concept is_spawner = std::is_same_v<std::remove_cvref_t<T>, mpicxx::single_spawner>
                      || std::is_same_v<std::remove_cvref_t<T>, mpicxx::multiple_spawner>;

    /**
     * @brief @concept{ @ref is_info<T> }
     *        Concept that describes the @ref mpicxx::info class.
     * @tparam T the compared to type
     */
    template <typename T>
    concept is_info = std::is_same_v<std::remove_cvref_t<T>, mpicxx::info>;
    ///@}

}

#endif // MPICXX_CONCEPTS_HPP