/**
 * @file test/info/iterators/reverse_iterator.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::rbegin() and @ref mpicxx::info::rend() member functions provided by the
 * @ref mpicxx::info class.
 * @details Testsuite: *IteratorsTest*
 * | test case name       | test case description                                       |
 * |:---------------------|:------------------------------------------------------------|
 * | ReverseIterator      | check for the correct iterator types                        |
 * | ReverseIteratorEmpty | check whether `rbegin() == rend()` for an empty info object |
 * | NullReverseIterator  | info object referring to MPI_INFO_NULL (death test)         |
 */

#include <type_traits>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


template <typename T, typename U>
constexpr bool check_iterator_type(U) { return std::is_same_v<T, U>; }

TEST(IteratorsTest, ReverseIterator) {
    // create info object
    mpicxx::info info;

    // check returned types
    EXPECT_TRUE(check_iterator_type<mpicxx::info::reverse_iterator>(info.rbegin()));
    EXPECT_TRUE(check_iterator_type<mpicxx::info::reverse_iterator>(info.rend()));
}

TEST(IteratorsTest, ReverseIteratorEmpty) {
    // create empty info object
    mpicxx::info info;

    // rbegin and rend should be the same
    EXPECT_EQ(info.rbegin(), info.rend());
}

TEST(IteratorsDeathTest, NullReverseIterator) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling rbegin() or rend() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] mpicxx::info::reverse_iterator it;
    ASSERT_DEATH( it = info.rbegin() , "");
    ASSERT_DEATH( it = info.rend() , "");
}