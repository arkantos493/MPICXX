/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-20
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines utility functions used in the mpicxx library.
 */

#ifndef MPICXX_UTILITY_HPP
#define MPICXX_UTILITY_HPP

#include <functional>
#include <type_traits>

namespace mpicxx::detail {

    /// @name utility functions checking if all values in a parameter pack fulfill a predicate
    ///@{
    /**
     * @brief Checks whether invoking the predicate @p pred with @p t and @p u evaluates to `true`.
     * @tparam BinaryOp a binary function (returning a `bool`)
     * @tparam T an arbitrary type
     * @tparam U an arbitrary type
     * @param[in] pred the binary function
     * @param[in] t first argument
     * @param[in] u second argument
     * @return `true` if `pred(t, u)` evaluates to `true`, otherwise `false`
     * @nodiscard
     */
    template <typename BinaryOp, typename T, typename U>
    [[nodiscard]]
    constexpr bool all_same(BinaryOp pred, const T& t, const U& u)
            noexcept(std::is_nothrow_invocable_r_v<bool, BinaryOp, T, U>)
            requires std::is_invocable_r_v<bool, BinaryOp, T, U>
    {
        return std::invoke(pred, t, u);
    }
    /**
     * @brief Checks whether the results of invoking the predicate @p pred with all parameters (@p t, @p u and @p args) compare equal.
     * @details Evaluates: `pred(a, b) == pred(b, c) == ... == pred(y, z)`.
     * @tparam BinaryOp a binary function (returning a `bool`)
     * @tparam T an arbitrary type
     * @tparam U an arbitrary type
     * @tparam Args a parameter pack of arbitrary types and size
     * @param[in] pred the binary function
     * @param[in] t first argument
     * @param[in] u second argument
     * @param[in] args remaining arguments
     * @return `true` if all results compare equal, `false` otherwise
     * @nodiscard
     */
    template <typename BinaryOp, typename T, typename U, typename... Args>
    [[nodiscard]]
    constexpr bool all_same(BinaryOp pred, const T& t, const U& u, const Args&... args)
            noexcept(std::is_nothrow_invocable_r_v<bool, BinaryOp, T, U>)
            requires std::is_invocable_r_v<bool, BinaryOp, T, U>
    {
        return std::invoke(pred, t, u) && all_same(pred, u, args...);
    }

    /**
     * @brief Checks whether the results of invoking the predicate @p pred with @p t and @p u compare equal.
     * @tparam UnaryOp an unary function
     * @tparam T an arbitrary type
     * @tparam U an arbitrary type
     * @param[in] pred the unary function
     * @param[in] t first argument
     * @param[in] u second argument
     * @return `true` if `pred(t) == pred(u)`, otherwise `false`
     * @nodiscard
     */
    template <typename UnaryOp, typename T, typename U>
    [[nodiscard]]
    constexpr bool all_same(UnaryOp pred, const T& t, const U& u)
            noexcept(std::is_nothrow_invocable_v<UnaryOp, T> && std::is_nothrow_invocable_v<UnaryOp, U>)
    {
        return std::invoke(pred, t) == std::invoke(pred, u);
    }

    /**
     * @brief Checks whether the results of invoking the predicate @p pred with all parameters (@p t, @p u and @p args) compare equal.
     * @tparam UnaryOp an unary function
     * @tparam T an arbitrary type
     * @tparam U an arbitrary type
     * @tparam Args a parameter pack of arbitrary types
     * @param[in] pred the unary function
     * @param[in] t first argument
     * @param[in] u second argument
     * @param[in] args remaining arguments
     * @return `true` if all results compare equal, otherwise `false`
     * @nodiscard
     */
    template <typename UnaryOp, typename T, typename U, typename... Args>
    [[nodiscard]]
    constexpr bool all_same(UnaryOp pred, const T& t, const U& u, const Args&... args)
            noexcept(std::is_nothrow_invocable_v<UnaryOp, T> && std::is_nothrow_invocable_v<UnaryOp, U>)
    {
        return (std::invoke(pred, t) == std::invoke(pred, u)) && all_same(pred, u, args...);
    }

    /**
     * @brief Checks whether all results of invoking @p pred compare equal.
     *        Since only one parameter is given, this function returns always `true`.
     * @tparam Op a function predicate
     * @tparam T the argument type
     * @param[in] pred the function
     * @param[in] t the argument
     * @return always `true`
     * @nodiscard
     */
    template <typename Op, typename T>
    [[nodiscard]]
    constexpr bool all_same(Op pred, const T& t) noexcept { return true; }
    ///@}

}

#endif // MPICXX_UTILITY_HPP