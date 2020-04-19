/**
 * @file include/mpicxx/detail/utility.hpp
 * @author Marcel Breyer
 * @date 2020-04-19
 *
 * @brief Defines utility functions used in the mpicxx library.
 */

#ifndef MPICXX_UTILITY_HPP
#define MPICXX_UTILITY_HPP

#include <type_traits>
#include <functional>


namespace mpicxx::detail {

    // TODO 2020-04-19 02:09 breyerml: comment, revise
    template <typename BinaryOp, typename T, typename U>
    requires std::is_same_v<std::invoke_result_t<BinaryOp, T, U>, bool>
    bool all_same(BinaryOp pred, const T& t, const U& u) {
        return std::invoke(pred, t, u);
    }

    template <typename BinaryOp, typename T, typename U, typename... Args>
    requires std::is_same_v<std::invoke_result_t<BinaryOp, T, U>, bool>
    bool all_same(BinaryOp pred, const T& t, const U& u, const Args&... args) {
        return std::invoke(pred, t, u) && all_same(pred, u, args...);
    }


    template <typename UnaryOp, typename T, typename U>
    bool all_same(UnaryOp pred, const T& t, const U& u) {
        return std::invoke(pred, t) == std::invoke(pred, u);
    }

    template <typename UnaryOp, typename T, typename U, typename... Args>
    bool all_same(UnaryOp pred, const T& t, const U& u, const Args&... args) {
        return (std::invoke(pred, t) == std::invoke(pred, u)) && all_same(pred, u, args...);
    }


    template <typename Op, typename T>
    bool all_same(Op pred, const T& t) { return true; }

}

#endif //MPICXX_UTILITY_HPP
