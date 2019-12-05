/**
 * @file concepts.hpp
 * @author Marcel Breyer
 * @date 2019-12-05
 *
 * @brief Defines various concepts to narrow down possible template instantiations.
 */

#ifndef MPICXX_CONCEPTS_HPP
#define MPICXX_CONCEPTS_HPP

namespace mpicxx::detail {

    /**
     * @brief Concepts that describes every "string" type, i.e. `std::string`, `char*` and `char[]`.
     * @tparam T
     */
    template <typename T>
    concept String = std::is_same_v<std::string, std::remove_cvref_t<T>> || std::is_convertible_v<std::decay_t<T>, const char*>;

}

#endif // MPICXX_CONCEPTS_HPP
