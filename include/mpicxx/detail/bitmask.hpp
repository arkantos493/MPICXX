/**
 * @file
 * @author Marcel Breyer
 * @date 2020-09-01
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines utility functions to treat enums or enum classes as bitmasks.
 */

#ifndef MPICXX_BITMASK_HPP
#define MPICXX_BITMASK_HPP

#include <bit>
#include <concepts>
#include <type_traits>

#define MPICXX_DEFINE_ENUM_UNARY_BITWISE_OPERATORS(Enum, Op) \
[[nodiscard]]                                                \
inline Enum operator Op (const Enum lhs) noexcept {          \
    using utype = std::underlying_type_t<Enum>;              \
    return static_cast<Enum>( Op static_cast<utype>(lhs) );  \
}                                                            \

#define MPICXX_DEFINE_ENUM_BINARY_BITWISE_OPERATORS(Enum, Op)                       \
[[nodiscard]]                                                                       \
inline Enum operator Op (const Enum lhs, const Enum rhs) noexcept {                 \
    using utype = std::underlying_type_t<Enum>;                                     \
    return static_cast<Enum>( static_cast<utype>(lhs) Op static_cast<utype>(rhs) ); \
}                                                                                   \
inline Enum& operator Op##=(Enum& lhs, const Enum rhs) noexcept {                   \
    lhs = lhs Op rhs;                                                               \
    return lhs;                                                                     \
}                                                                                   \

#define MPICXX_DEFINE_ENUM_BITWISE_OPERATORS(Enum)   \
MPICXX_DEFINE_ENUM_UNARY_BITWISE_OPERATORS(Enum, ~)  \
MPICXX_DEFINE_ENUM_BINARY_BITWISE_OPERATORS(Enum, |) \
MPICXX_DEFINE_ENUM_BINARY_BITWISE_OPERATORS(Enum, &) \
MPICXX_DEFINE_ENUM_BINARY_BITWISE_OPERATORS(Enum, ^) \

namespace mpicxx::detail::bitmask {

    /**
     * @brief @concept{ @ref has_bitwise_operators }
     *        Concept that describes a type implementing all bitwise operators and hence fulfilling the
     *        [*BitmaskType*](https://en.cppreference.com/w/cpp/named_req/BitmaskType).
     * @tparam T the compared to type
     */
    template <typename T>
    concept has_bitwise_operators = requires (T lhs, T rhs) {
        { ~lhs }       -> std::convertible_to<typename std::remove_cvref_t<T>>;
        { lhs | rhs }  -> std::convertible_to<typename std::remove_cvref_t<T>>;
        { lhs & rhs }  -> std::convertible_to<typename std::remove_cvref_t<T>>;
        { lhs ^ rhs }  -> std::convertible_to<typename std::remove_cvref_t<T>>;
        { lhs |= rhs } -> std::convertible_to<typename std::add_lvalue_reference_t<typename std::remove_cvref_t<T>>>;
        { lhs &= rhs } -> std::convertible_to<typename std::add_lvalue_reference_t<typename std::remove_cvref_t<T>>>;
        { lhs ^= rhs } -> std::convertible_to<typename std::add_lvalue_reference_t<typename std::remove_cvref_t<T>>>;
    };
    /**
     * @brief @concept{ @ref is_bitwise_enum<T> }
     *        Concept that describes an enum or enum class which fulfills the @ref mpicxx::detail::bitmask::has_bitwise_operators concept.
     * @tparam T the compared to type
     */
    template <typename T>
    concept is_bitwise_enum = std::is_enum_v<T> && has_bitwise_operators<T>;
    

    /**
     * @brief Tests whether the bit/bits represented by @p bit is/are set in @p bitmask.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[in] bitmask the bitmask to test
     * @param[in] bit the bit/bits to test
     * @return `true` if the bit/bits is/are set, otherwise `false`
     * @nodiscard
     */
    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr bool test(const T bitmask, const T bit) noexcept {
        return (bitmask & bit) == bit;
    }
    /**
     * @brief Checks whether all bits in @p bitmask are set to 0.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[in] bitmask the bitmask to check
     * @return `true` if all bits are set to 0
     * @nodiscard
     */
    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr bool none(const T bitmask) noexcept {
        return static_cast<std::underlying_type_t<T>>(bitmask) == 0;
    }
    /**
     * @brief Checks whether any bit in @p bitmask is set to 1.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[in] bitmask the bitmask to check
     * @return `true` if any bit is set to 1
     * @nodiscard
     */
    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr bool any(const T bitmask) noexcept {
        return static_cast<std::underlying_type_t<T>>(bitmask) != 0;
    }
    /**
     * @brief Checks whether all bits in @p bitmask are set to 1.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[in] bitmask the bitmask to check
     * @return  `true` if all bits are set to 1
     * @nodiscard
     */
    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr bool all(const T bitmask) noexcept {
        return static_cast<std::underlying_type_t<T>>(bitmask) == ~static_cast<std::underlying_type_t<std::remove_reference_t<T>>>(0);
    }
    /**
     * @brief Counts how many bits in @p bitmask are set to 1.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[in] bitmask the bitmask to check
     * @return the number of set bits in @p bitmask
     * @nodiscard
     */
    template <is_bitwise_enum T>
    [[nodiscard]]
    constexpr int count(const T bitmask) noexcept {
        return std::popcount(static_cast<std::underlying_type_t<T>>(bitmask));
    }
    /**
     * @brief Sets all bits in @p bitmask to 1.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[out] bitmask the bitmask to set
     */
    template <is_bitwise_enum T>
    constexpr void set(T& bitmask) noexcept {
        bitmask = static_cast<T>(~static_cast<std::underlying_type_t<std::remove_reference_t<T>>>(0));
    }
    /**
     * @brief Sets the bit/bits represented by @p bit in @p bitmask to 1.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[inout] bitmask the bitmask to set
     * @param[in] bit the bit/bits to set
     */
    template <is_bitwise_enum T>
    constexpr void set(T& bitmask, const T bit) noexcept {
        bitmask |= bit;
    }
    /**
     * @brief Sets all bits in @p bitmask to 0.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[out] bitmask the bitmask to reset
     */
    template <is_bitwise_enum T>
    constexpr void reset(T& bitmask) noexcept {
        bitmask = static_cast<T>(0);
    }
    /**
     * @brief Sets the bit/bits represented by @p bit in @p bitmask to 0.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[inout] bitmask the bitmask to reset
     * @param[in] bit the bit/bits to reset
     */
    template <is_bitwise_enum T>
    constexpr void reset(T& bitmask, const T bit) noexcept {
        bitmask &= ~bit;
    }
    /**
     * @brief Flips all bits in @p bitmask.
     * @details Maps: 0 -> 1 and 1 -> 0.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[out] bitmask the bitmask to flip
     */
    template <is_bitwise_enum T>
    constexpr void flip(T& bitmask) noexcept {
        bitmask = ~bitmask;
    }
    /**
     * @brief Flips the bit/bits represented by @p bit in @p bitmask.
     * @tparam T must meet the @ref mpicxx::detail::bitmask::is_bitwise_enum requirements
     * @param[inout] bitmask the bitmask to flip
     * @param[in] bit the bit/bits to flip
     */
    template <is_bitwise_enum T>
    constexpr void flip(T& bitmask, const T bit) noexcept {
        bitmask ^= bit;
    }

}

#endif // MPICXX_BITMASK_HPP