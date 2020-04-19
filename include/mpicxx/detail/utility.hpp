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

namespace mpicxx::detail {

    // TODO 2020-04-19 02:09 breyerml: comment, revise
    template <typename Pred, typename T, typename U>
    requires std::is_same_v<std::result_of_t<Pred(T, U)>, bool>
    bool all_same(Pred pred, const T& t, const U& u) {
        return pred(t, u);
    }

    template <typename Pred, typename T, typename U, typename... Args>
    requires std::is_same_v<std::result_of_t<Pred(T, U)>, bool>
    bool all_same(Pred pred, const T& t, const U& u, const Args&... args) {
        return pred(t, u) && all_same(pred, u, args...);
    }

}

#endif //MPICXX_UTILITY_HPP
