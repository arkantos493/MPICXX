/**
 * @file info/iterators/iterator.cpp
 * @author Marcel Breyer
 * @date 2020-02-03
 *
 * @brief Test cases for the @ref mpicxx::info::begin() and @ref mpicxx::info::end() member functions provided by the
 * @ref mpicxx::info class.
 * @details Testsuite: *IteratorsTest*
 * | test case name    | test case description                                     |
 * |:------------------|:----------------------------------------------------------|
 * | Iterator          | check for the correct iterator types                      |
 * | IteratorEmpty     | check whether `begin() == end()` for an empty info object |
 * | MovedFromIterator | info object in the moved-from state (death test)          |
 */

#include <type_traits>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


template <typename T, typename U>
constexpr bool check_iterator_type(U) { return std::is_same_v<T, U>; }

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

TEST(IteratorsDeathTest, MovedFromIterator) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling begin() or end() on an info object in the moved-from state is illegal
    [[maybe_unused]] mpicxx::info::iterator it;
    ASSERT_DEATH( it = info.begin() , "");
    ASSERT_DEATH( it = info.end() , "");
}