/**
 * @file string.hpp
 * @author Marcel Breyer
 * @date 2019-12-05
 *
 * @brief Implements convenience functions for a more uniform usage of `std::string` and c-style strings.
 */

#ifndef MPICXX_STRING_HPP
#define MPICXX_STRING_HPP

#include <type_traits>

namespace mpicxx::utility {

    // ---------------------------------------------------------------------------------------------------------- //
    //                                             to c-style strings                                             //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Convert a constant `std::string` to a `const char*`.
     * @param str the `std::string`
     * @return null-terminated c-style string, i.e. `const char*`
     */
    inline const char* to_c_string(const std::string& str) {
        return str.data();
    }
    /**
     * @brief Convert a `std::string` to a `char*`.
     * @param str the `std::string`
     * @return null-terminated c-style string, i.e. `char*`
     */
    inline char* to_c_string(std::string& str) {
        return str.data();
    }
    /**
     * @brief Convert a `const char*` to a `const char*`, i.e. directly returns @p str.
     * @param str the `const char*`
     * @return null-terminated c-style string, i.e. `const char*`
     */
    inline const char* to_c_string(const char* str) {
        return str;
    }
    /**
     * @brief Convert a `char*` to a `char*`, i.e. directly returns @p str.
     * @param str the `char*`
     * @return null-terminated c-style string, i.e. `char*`
     */
    inline char* to_c_string(char* str) {
        return str;
    }


    // ---------------------------------------------------------------------------------------------------------- //
    //                                                    size                                                    //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Returns the size of a `std::string`.
     * @details Does **not** include the null-terminator.
     * @param str the `std::string`
     * @return the size of the `std::string`
     */
    inline std::size_t string_size(const std::string& str, std::size_t) {
        return str.size();
    }
    /**
     * @brief Returns the size of a `const char[N]`
     * @tparam N the size of the array
     * @return the size of the array
     *
     * @pre the array must include the null-terminator (i.e. returns N - 1)
     */
    template <std::size_t N>
    inline std::size_t string_size(const char(&)[N], std::size_t) {
        return N - 1;
    }
    /**
     * @brief Returns the size of the `const char*`.
     * @details At most @p max_len chars are observed. If no null-terminator is found up to this point, @p max_len is returned.
     * @tparam char_ptr the `const char*`, needed to SFINAE away potential calls with `const char[]` to another function
     * @param str the `const char*`
     * @param max_len the maximal number of looked at elements
     * @return the size of the c-string literal or @p max_len iff the null-terminator is **not** present
     *
     * @pre a function call is valid iff @p str contains the null-terminator or is greater than @p max_len
     */
    template <typename char_ptr>
    inline std::size_t string_size(const char_ptr str, std::size_t max_len) requires (!std::is_array_v<char_ptr>) {
        for (std::size_t i = 0; i < max_len; ++i) {
            if (str[i] == '\0') {
                return i;
            }
        }
        return max_len;
    }

}

#endif // MPICXX_STRING_HPP
