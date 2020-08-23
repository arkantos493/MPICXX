/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-23
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines utility functions to treat enums or enum classes as bitmasks.
 */

#ifndef MPICXX_BITMASK_HPP
#define MPICXX_BITMASK_HPP

#include <bit>
#include <concepts>
#include <type_traits>

namespace mpicxx::detail::bitmask {
    
    template <typename T>
    concept has_bitwise_operators = requires (T lhs, T rhs) {
        { ~lhs }       -> std::convertible_to<typename std::remove_cvref_t<T>>;
        { lhs | rhs }  -> std::convertible_to<typename std::remove_cvref_t<T>>;
        { lhs & rhs }  -> std::convertible_to<typename std::remove_cvref_t<T>>;
        { lhs ^ rhs }  -> std::convertible_to<typename std::remove_cvref_t<T>>;
        { lhs |= rhs } -> std::convertible_to<typename std::remove_cvref_t<T>&>;
        { lhs &= rhs } -> std::convertible_to<typename std::remove_cvref_t<T>&>;
        { lhs ^= rhs } -> std::convertible_to<typename std::remove_cvref_t<T>&>;
    };

    template <typename T>
    concept is_bitwise_enum = std::is_enum_v<T> && has_bitwise_operators<T>;
    

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr bool test(const T bitmask, const T bit) noexcept {
        return (bitmask & bit) == bit;
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr bool none(const T bitmask) noexcept {
        return static_cast<std::underlying_type_t<T>>(bitmask) == 0;
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr bool any(const T bitmask) noexcept {
        return static_cast<std::underlying_type_t<T>>(bitmask) != 0;
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr bool all(const T bitmask) noexcept {
        return static_cast<std::underlying_type_t<T>>(bitmask) == ~static_cast<std::underlying_type_t<std::remove_reference_t<T>>>(0);
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr int count(const T bitmask) noexcept {
        return std::popcount(static_cast<std::underlying_type_t<T>>(bitmask));
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr T set(const T) noexcept {
        return static_cast<T>(~static_cast<std::underlying_type_t<std::remove_reference_t<T>>>(0));
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr T set(const T bitmask, const T bit) noexcept {
        return bitmask | bit;
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr T reset(const T) noexcept {
        return static_cast<T>(0);
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr T reset(const T bitmask, const T bit) noexcept {
        return bitmask & (~bit);
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr T flip(const T bitmask) noexcept {
        return ~bitmask;
    }

    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr T flip(const T bitmask, const T bit) noexcept {
        return bitmask ^ bit;
    }

}

#endif // MPICXX_BITMASK_HPP
