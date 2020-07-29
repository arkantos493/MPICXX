/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::begin() and @ref mpicxx::info::end() member functions provided by the
 *        @ref mpicxx::info class.
 * @details Testsuite: *IteratorsTest*
 * | test case name | test case description                                                                                                    |
 * |:---------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | Iterator       | check for the correct iterator types                                                                                     |
 * | IteratorEmpty  | check whether `begin() == end()` for an empty info object                                                                |
 * | NullIterator   | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <type_traits>

namespace {

    template <typename T, typename U>
    constexpr bool check_iterator_type(U) { return std::is_same_v<T, U>; }

}

TEST(IteratorsTest, Iterator) {
    // create info object
    mpicxx::info info;

    // check returned types
    EXPECT_TRUE(check_iterator_type<mpicxx::info::iterator>(info.begin()));
    EXPECT_TRUE(check_iterator_type<mpicxx::info::iterator>(info.end()));
}

TEST(IteratorsTest, IteratorEmpty) {
    // create empty info object
    mpicxx::info info;

    // begin and end should be the same
    EXPECT_EQ(info.begin(), info.end());
}

TEST(IteratorsDeathTest, NullIterator) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling begin() or end() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] mpicxx::info::iterator it;
    ASSERT_DEATH( it = info.begin() , "");
    ASSERT_DEATH( it = info.end() , "");
}