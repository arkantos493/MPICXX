/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::rbegin() const, @ref mpicxx::info::rend() const, @ref mpicxx::info::crbegin() const and
 *        @ref mpicxx::info::crend() const member functions provided by the @ref mpicxx::info class.
 * @details Testsuite: *IteratorsTest*
 * | test case name            | test case description                                                                                                    |
 * |:--------------------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | ConstReverseIterator      | check for the correct iterator types                                                                                     |
 * | ConstReverseIteratorEmpty | check whether `crbegin() == crend()` for an empty info object                                                            |
 * | NullConstReverseIterator  | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <type_traits>

namespace {

    template <typename T, typename U>
    constexpr bool check_iterator_type(U) { return std::is_same_v<T, U>; }

}

TEST(IteratorsTest, ConstReverseIterator) {
    // create info object
    mpicxx::info info;

    // check returned types
    EXPECT_TRUE(check_iterator_type<mpicxx::info::const_reverse_iterator>(info.crbegin()));
    EXPECT_TRUE(check_iterator_type<mpicxx::info::const_reverse_iterator>(info.crend()));

    // create const info object
    const mpicxx::info const_info;

    // check returned types
    EXPECT_TRUE(check_iterator_type<mpicxx::info::const_reverse_iterator>(const_info.rbegin()));
    EXPECT_TRUE(check_iterator_type<mpicxx::info::const_reverse_iterator>(const_info.rend()));
}

TEST(IteratorsTest, ConstReverseIteratorEmpty) {
    // create empty info object
    mpicxx::info info;

    // rbegin and rend should be the same
    EXPECT_EQ(info.crbegin(), info.crend());

    // create empty const info object
    const mpicxx::info const_info;

    // rbegin and rend should be the same
    EXPECT_EQ(const_info.rbegin(), const_info.rend());
}

TEST(IteratorsDeathTest, NullConstReverseIterator) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling crbegin() or crend() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] mpicxx::info::const_reverse_iterator it;
    ASSERT_DEATH( it = info.crbegin() , "");
    ASSERT_DEATH( it = info.crend() , "");

    // create const null info object
    const mpicxx::info const_info(MPI_INFO_NULL, false);

    // calling rbegin() const or rend() const on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( it = const_info.rbegin() , "");
    ASSERT_DEATH( it = const_info.rend() , "");
}