/**
 * @file concepts.hpp
 * @author Marcel Breyer
 * @date 2019-12-09
 *
 * @brief Defines various concepts to narrow down possible template instantiations.
 */

#ifndef MPICXX_CONCEPTS_HPP
#define MPICXX_CONCEPTS_HPP

namespace mpicxx::detail {

    /**
     * @brief Concepts that describes every "string" type, i.e. `std::string`, `std::string_view`, `const char*` and `char[]`.
     * @tparam T
     */
    template <typename T>
    concept string = std::is_constructible_v<std::string, T>; // TODO 2020-01-22 18:07 marcel: is_constructible_v<std::string_view, T> ???

}

#endif // MPICXX_CONCEPTS_HPP
