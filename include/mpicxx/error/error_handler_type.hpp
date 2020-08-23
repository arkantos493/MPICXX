/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-23
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines the error handler type for which the @ref mpicxx::error_handler can be used.
 */

#ifndef MPICXX_ERROR_HANDLER_TYPE_HPP
#define MPICXX_ERROR_HANDLER_TYPE_HPP

#include <mpicxx/detail/bitmask.hpp>

#include <array>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>

namespace mpicxx {

    /**
     * @brief Enum class for the different types of error handlers provided by MPI.
     */
    enum class error_handler_type {
        /** error handler type for communicators */
        comm = 1 << 0,
        /** error handler type for files*/
        file = 1 << 1,
        /** error handler type for windows */
        win  = 1 << 2
    };


    /// @name mpicxx::error_handler_type bitwise arithmetic operators
    /**
     * @brief Bitwise NOT operator overload for the @ref mpicxx::error_handler_type enum class.
     * @param eht the enum class value
     * @return the bitwise NOT value of @p eht
     * @nodiscard
     */
    [[nodiscard]]
    inline error_handler_type operator~(const error_handler_type eht) {
        return static_cast<error_handler_type>( ~static_cast<std::underlying_type_t<error_handler_type>>(eht) );
    }
    /**
     * @brief Bitwise OR operator overload for the @ref mpicxx::error_handler_type enum class.
     * @param lhs an enum class value
     * @param rhs an enum class value
     * @return the bitwise OR value of @p lhs and @p rhs
     * @nodiscard
     */
    [[nodiscard]]
    inline error_handler_type operator|(const error_handler_type lhs, const error_handler_type rhs) {
        using type = std::underlying_type_t<error_handler_type>;
        return static_cast<error_handler_type>( static_cast<type>(lhs) | static_cast<type>(rhs) );
    }
    /**
     * @brief Bitwise AND operator overload for the @ref mpicxx::error_handler_type enum class.
     * @param lhs an enum class value
     * @param rhs an enum class value
     * @return the bitwise AND value of @p lhs and @p rhs
     * @nodiscard
     */
    [[nodiscard]]
    inline error_handler_type operator&(const error_handler_type lhs, const error_handler_type rhs) {
        using type = std::underlying_type_t<error_handler_type>;
        return static_cast<error_handler_type>( static_cast<type>(lhs) & static_cast<type>(rhs) );
    }
    /**
     * @brief Bitwise XOR operator overload for the @ref mpicxx::error_handler_type enum class.
     * @param lhs an enum class value
     * @param rhs an enum class value
     * @return the bitwise XOR value of @p lhs and @p rhs
     * @nodiscard
     */
    [[nodiscard]]
    inline error_handler_type operator^(const error_handler_type lhs, const error_handler_type rhs) {
        using type = std::underlying_type_t<error_handler_type>;
        return static_cast<error_handler_type>( static_cast<type>(lhs) ^ static_cast<type>(rhs) );
    }
    /**
     * @brief Compound bitwise OR operator overload for the @ref mpicxx::error_handler_type enum class.
     * @param lhs an enum class value
     * @param rhs an enum class value
     * @return @p lhs containing the bitwise OR value of @p lhs and @p rhs
     */
    inline error_handler_type& operator|=(error_handler_type& lhs, const error_handler_type rhs) {
        lhs = lhs | rhs;
        return lhs;
    }
    /**
     * @brief Compound bitwise AND operator overload for the @ref mpicxx::error_handler_type enum class.
     * @param lhs an enum class value
     * @param rhs an enum class value
     * @return @p lhs containing the bitwise AND value of @p lhs and @p rhs
     */
    inline error_handler_type& operator&=(error_handler_type& lhs, const error_handler_type rhs) {
        lhs = lhs & rhs;
        return lhs;
    }
    /**
     * @brief Compound bitwise XOR operator overload for the @ref mpicxx::error_handler_type enum class.
     * @param lhs an enum class value
     * @param rhs an enum class value
     * @return @p lhs containing the bitwise XOR value of @p lhs and @p rhs
     */
    inline error_handler_type& operator^=(error_handler_type& lhs, const error_handler_type rhs) {
        lhs = lhs ^ rhs;
        return lhs;
    }
    ///@}


    /// @name @ref mpicxx::error_handler_type conversion functions
    ///@{
    /**
     * @brief Stream-insertion operator overload for the @ref mpicxx::error_handler_type enum class.
     * @details Only
     * @param[inout] out an output stream
     * @param[in] eht the enum class value
     * @return the output stream
     */
    inline std::ostream& operator<<(std::ostream& out, const error_handler_type eht) {
        std::array<const char*, 3> strs;
        std::size_t idx = 0;
        if (detail::bitmask::test(eht, error_handler_type::comm)) {
            strs[idx++] = "COMM";
        }
        if (detail::bitmask::test(eht, error_handler_type::file)) {
            strs[idx++] = "FILE";
        }
        if (detail::bitmask::test(eht, error_handler_type::win)) {
            strs[idx++] = "WIN";
        }
        return out << fmt::format("{}", fmt::join(strs.begin(), strs.begin() + idx, " | "));
    }
    /**
     * @brief Overload of the @ref mpicxx::to_string(T&&) function for the @ref mpicxx::error_handler_type enum class.
     * @param[in] eht the enum class value
     * @return the [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) representation of the enum class value
     * @nodiscard
     */
    [[nodiscard]]
    inline std::string to_string(const error_handler_type eht) {
        return fmt::format("{}", eht);
    }

    /**
     * @brief Converts the given string to the respective @ref mpicxx::error_handler_type value.
     * @details Expects the string value to be one of `"COMM"`, `"FILE"` or `"WIN"`.
     * @param[in] sv the enum value represented as a string
     * @return the @ref mpicxx::error_handler_type representation of @p sv
     * @nodiscard
     *
     * @throws std::invalid_argument if the given value can't be converted to a @ref mpicxx::error_handler_type value
     */
    [[nodiscard]]
    inline error_handler_type error_handler_type_from_string(const std::string_view sv) {
        error_handler_type eht = static_cast<error_handler_type>(0);
        std::size_t pos = 0;
        std::size_t next = 0;
        do {
            next = sv.find("|", pos);
            const std::string_view substr = detail::trim(sv.substr(pos, next));

            if (substr.compare("COMM") == 0) {
                eht = detail::bitmask::set(eht, error_handler_type::comm);
            } else if (substr.compare("FILE") == 0) {
                eht = detail::bitmask::set(eht, error_handler_type::file);
            } else if (substr.compare("WIN") == 0) {
                eht = detail::bitmask::set(eht, error_handler_type::win);
            }

            pos = next + 1;
        } while (next != std::string_view::npos);

        if (static_cast<std::underlying_type_t<error_handler_type>>(eht) == 0) {
            throw std::invalid_argument(fmt::format("Can't convert \"{}\" to mpicxx::error_handler_type!", sv));
        }

        return eht;
    }
    /**
     * @brief Stream-extraction operator overload for the @ref mpicxx::error_handler_type enum class.
     * @details Sets the [`std::ios::failbit`](https://en.cppreference.com/w/cpp/io/ios_base/iostate) if the given value can't be converted
     *          to a @ref mpicxx::error_handler_type value.
     * @param[inout] in an input stream
     * @param[out] eht the enum class
     * @return the input stream
     */
    inline std::istream& operator>>(std::istream& in, error_handler_type& eht) {
        try {
            std::string str;
            in >> str;
            eht = error_handler_type_from_string(str);
        } catch (const std::exception&) {
            in.setstate(std::ios::failbit);
        }
        return in;
    }
    ///@}

}

#endif // MPICXX_ERROR_HANDLER_TYPE_HPP
