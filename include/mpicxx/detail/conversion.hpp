/**
 * @file include/mpicxx/detail/conversion.hpp
 * @author Marcel Breyer
 * @date 2020-06-28
 *
 * @brief Defines conversion functions used in the mpicxx library.
 */

#ifndef MPICXX_CONVERSION_HPP
#define MPICXX_CONVERSION_HPP

#include <cstring>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <mpicxx/detail/concepts.hpp>


namespace mpicxx {

    /// @name conversion functions to std::string
    ///@{
    namespace detail::adl_helper {
        using std::to_string;
        /**
         * @brief Performs an ADL for the [`to_string`](https://en.cppreference.com/w/cpp/string/basic_string/to_string) function.
         * @tparam T the type to convert
         * @param[in] arg the value to convert
         * @return the [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) representation of @p arg (`[[nodiscard]]`)
         */
        template <typename T>
        inline std::string as_string(T&& arg) {
            return to_string(std::forward<T>(arg));
        }
    }

    /**
     * @brief Calls the @ref detail::adl_helper::as_string(T&&) function, which performs an ADL for the type `T`.
     * @tparam T the type to convert
     * @param[in] arg the value to convert
     * @return the [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) representation of @p arg (`[[nodiscard]]`)
     */
    template <typename T>
    inline std::string to_string(T&& arg) {
        return detail::adl_helper::as_string(std::forward<T>(arg));
    }
    ///@}

}

namespace mpicxx::detail {

    /// @name custom, internally used, C++20 concepts
    ///@{
    /**
     * @brief Concept that describes a type that can be converted to a
     *        [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) using the
     *        [`to_string`](https://en.cppreference.com/w/cpp/string/basic_string/to_string) function.
     * @tparam T the compared to type
     */
    template <typename T>
    concept has_to_string = requires (T t) { std::to_string(t); } || requires (T t) { to_string(t); };
    /**
     * @brief Concept that describes a type that can be converted to a
     *        [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) using the
     *        [`operator<<`](https://en.cppreference.com/w/cpp/language/operators) overload.
     * @tparam T the compared to type
     */
    template <typename T>
    concept has_ostringstream = requires (T t) { std::declval<std::ostringstream>() << t; };
    ///@}

    /// @name conversion functions to std::string
    ///@{
    /**
     * @brief Tries to convert the given @p arg to a [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
     * @details Tries to use the following ways to convert @p arg (in this order):
     *          -# If T is a `bool`, returns either `std::string("true")` or `std::string("false")`.
     *          -# If T is a `char`, returns `std::string(1, arg)`
     *          -# If T meets the @ref detail::is_string requirements, returns `std::string(std::forward<T>(arg))`.
     *          -# T can be converted using a [`to_string`](https://en.cppreference.com/w/cpp/string/basic_string/to_string) function.
     *          -# T can be converted using a [`operator<<`](https://en.cppreference.com/w/cpp/language/operators) overload.
     *
     *          If @p arg can't be convert using one of this ways, a compiler error is issued.
     * @tparam T the type to convert
     * @param[in] arg the value to convert
     * @return the [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) representation of @p arg (`[[nodiscard]]`)
     */
    template <typename T>
    [[nodiscard]]
    inline std::string convert_to_string(T&& arg) {
        using type = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<type, bool>) {
            // convert the given boolean to its std::string representation
            return arg ? std::string("true") : std::string("false");
        } else if constexpr (std::is_same_v<type, char>) {
            // convert single character to a std::string
            return std::string(1, arg);
        } else if constexpr (is_string<type>) {
            // convert a (const) char*, (const) char[] or (const) std::string to a string
            return std::string(std::forward<T>(arg));
        } else if constexpr (has_to_string<type>) {
            // convert the given arg to a std::string using the to_string method
            return mpicxx::to_string(std::forward<T>(arg));
        } else if constexpr (has_ostringstream<type>) {
            // convert the arg value to a std::string using an operator<< overload
            std::ostringstream os;
            os << std::forward<T>(arg);
            return os.str();
        } else {
            // unable to convert the given arg to a std::string
            constexpr bool dependent_false = !std::is_same_v<type, type>;
            static_assert(dependent_false, "Can't convert the given value to a std::string!");
        }
    }
    ///@}

    /// @name conversion functions
    ///@{
    /**
     * @brief Returns a pointer to the string @p str.
     * @details If @p str is a pointer type, returns @p str, otherwise calls [`std::data`](https://en.cppreference.com/w/cpp/iterator/data).
     * @tparam T must meet the @ref detail::is_string requirements
     * @param str the string to get the pointer to
     * @return the pointer to @p str (`[[nodiscard]]`)
     */
    template <is_string T>
    [[nodiscard]]
    constexpr auto convert_to_char_pointer(const T& str) noexcept {
        if constexpr (std::is_pointer_v<T>) {
            return str;
        } else {
            return std::data(str);
        }
    }
    /**
     * @brief Returns the size of the string @p str.
     * @tparam T must meet the @ref detail::is_string requirements
     * @param str the string to get the size from
     * @return the size of @p str (`[[nodiscard]]`)
     */
    template <is_string T>
    [[nodiscard]]
    constexpr std::size_t convert_to_string_size(const T& str) noexcept {
        if constexpr (std::is_pointer_v<std::decay_t<T>>) {
            return std::strlen(str);
        } else {
            return std::size(str);
        }
    }
    ///@}

}

#endif // MPICXX_CONVERSION_HPP