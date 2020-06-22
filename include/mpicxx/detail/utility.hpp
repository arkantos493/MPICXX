/**
 * @file include/mpicxx/detail/utility.hpp
 * @author Marcel Breyer
 * @date 2020-06-23
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
     * @tparam BinaryOp a binary function (returning a `bool`).
     * @tparam T an arbitrary type.
     * @tparam U an arbitrary type.
     * @param pred the binary function
     * @param t first argument
     * @param u second argument
     * @return `true` if `pred(t, u)` evaluates to `true`, otherwise `false` (`[[nodiscard]]`)
     */
    template <typename BinaryOp, typename T, typename U>
    requires std::is_same_v<std::invoke_result_t<BinaryOp, T, U>, bool>
    [[nodiscard]]
    inline bool all_same(BinaryOp pred, const T& t, const U& u) {
        return std::invoke(pred, t, u);
    }
    /**
     * @brief Checks whether the results of invoking the predicate @p pred with all parameters (@p t, @p u and @p args) compare equal.
     * @details Evaluates: `pred(a, b) == pred(b, c) == ... == pred(y, z)`.
     * @tparam BinaryOp a binary function (returning a `bool`).
     * @tparam T an arbitrary type.
     * @tparam U an arbitrary type.
     * @tparam Args a parameter pack of arbitrary types.
     * @param pred the binary function
     * @param t first argument
     * @param u second argument
     * @param args remaining arguments
     * @return `true` if all results compare equal, `false` otherwise (`[[nodiscard]]`)
     */
    template <typename BinaryOp, typename T, typename U, typename... Args>
    requires std::is_same_v<std::invoke_result_t<BinaryOp, T, U>, bool>
    [[nodiscard]]
    inline bool all_same(BinaryOp pred, const T& t, const U& u, const Args&... args) {
        return std::invoke(pred, t, u) && all_same(pred, u, args...);
    }

    /**
     * @brief Checks whether the results of invoking the predicate @p pred with @p t and @p u compare equal.
     * @tparam UnaryOp an unary function.
     * @tparam T an arbitrary type.
     * @tparam U an arbitrary type.
     * @param pred the unary function
     * @param t first argument
     * @param u second argument
     * @return `true` if `pred(t) == pred(u)`, otherwise `false` (`[[nodiscard]]`)
     */
    template <typename UnaryOp, typename T, typename U>
    [[nodiscard]]
    inline bool all_same(UnaryOp pred, const T& t, const U& u) {
        return std::invoke(pred, t) == std::invoke(pred, u);
    }
    /**
     * @brief Checks whether the results of invoking the predicate @p pred with all parameters (@p t, @p u and @p args) compare equal.
     * @tparam UnaryOp an unary function.
     * @tparam T an arbitrary type.
     * @tparam U an arbitrary type.
     * @tparam Args a parameter pack of arbitrary types.
     * @param pred the unary function
     * @param t first argument
     * @param u second argument
     * @param args remaining arguments
     * @return `true` if all results compare equal, otherwise `false` (`[[nodiscard]]`)
     */
    template <typename UnaryOp, typename T, typename U, typename... Args>
    [[nodiscard]]
    inline bool all_same(UnaryOp pred, const T& t, const U& u, const Args&... args) {
        return (std::invoke(pred, t) == std::invoke(pred, u)) && all_same(pred, u, args...);
    }

    /**
     * @brief Checks whether all results of invoking @p pred compare equal.
     *        Since only one parameter is given, this function returns always `true`.
     * @tparam Op a function predicate.
     * @tparam T the argument type.
     * @param pred the function
     * @param t the argument
     * @return always `true` (`[[nodiscard]]`)
     */
    template <typename Op, typename T>
    [[nodiscard]]
    inline bool all_same(Op pred, const T& t) { return true; }
    ///@}

}


#endif //MPICXX_UTILITY_HPP
