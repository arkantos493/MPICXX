/**
 * @file string.hpp
 * @author Marcel Breyer
 * @date 2019-11-24
 *
 * @brief Implements convenience functions for a more uniform usage of `std::string` and c-style strings.
 */

#ifndef MPICXX_STRING_HPP
#define MPICXX_STRING_HPP

namespace mpicxx::utility {
    // TODO 2019-11-24 20:12 marcel: add tests
    // ---------------------------------------------------------------------------------------------------------- //
    //                                             to c-style strings                                             //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Function to provide a uniform possibility to convert a `std::string` or `char*` to a c-style string.
     * @param str the given string
     * @return c-style string, i.e. `char*`
     */
    inline const char* to_c_string(const std::string& str) {
        std::cout << "const std::string&" << std::endl;
        return str.data();
    }
    /**
     * @copydoc to_c_string()
     */
    inline char* to_c_string(std::string& str) {
        std::cout << "std::string&" << std::endl;
        return str.data();
    }
    /**
     * @copydoc to_c_string()
     */
    inline const char* to_c_string(const char* str) {
        std::cout << "const char*" << std::endl;
        return str;
    }
    /**
     * @copydoc to_c_string()
     */
    inline char* to_c_string(char* str) {
        std::cout << "char*" << std::endl;
        return str;
    }


    // ---------------------------------------------------------------------------------------------------------- //
    //                                                    size                                                    //
    // ---------------------------------------------------------------------------------------------------------- //
    /**
     * @brief Function to provide a uniform possibility to get the size of a string.
     * @param str the given string
     * @return the size of the string
     */
    inline std::size_t string_size(const std::string& str) {
        return str.size();
    }
    /**
     * @copydoc string_size()
     */
    inline std::size_t string_size(const char* str) {
        return std::strlen(str);
    }
    /**
     * @copydoc string_size()
     * @tparam N the size of the c-style array
     */
    template <std::size_t N>
    inline std::size_t string_size(const char (&str)[N]) {
        return N;
    }

}

#endif // MPICXX_STRING_HPP
