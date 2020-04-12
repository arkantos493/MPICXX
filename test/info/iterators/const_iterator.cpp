/**
 * @file test/info/iterators/const_iterator.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::begin() const, @ref mpicxx::info::end() const, @ref mpicxx::info::cbegin() const and
 * @ref mpicxx::info::cend() const member functions provided by the @ref mpicxx::info class.
 * @details Testsuite: *IteratorsTest*
 * | test case name     | test case description                                       |
 * |:-------------------|:------------------------------------------------------------|
 * | ConstIterator      | check for the correct iterator types                        |
 * | ConstIteratorEmpty | check whether `cbegin() == cend()` for an empty info object |
 * | NullConstIterator  | info object referring to MPI_INFO_NULL (death test)         |
 */

#include <type_traits>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


template <typename T, typename U>
constexpr bool check_iterator_type(U) { return std::is_same_v<T, U>; }

TEST(IteratorsTest, ConstIterator) {
    // create info object
    mpicxx::info info;

    // check returned types
    EXPECT_TRUE(check_iterator_type<mpicxx::info::const_iterator>(info.cbegin()));
    EXPECT_TRUE(check_iterator_type<mpicxx::info::const_iterator>(info.cend()));

    // create const info object
    const mpicxx::info const_info;

    // check returned types
    EXPECT_TRUE(check_iterator_type<mpicxx::info::const_iterator>(const_info.begin()));
    EXPECT_TRUE(check_iterator_type<mpicxx::info::const_iterator>(const_info.end()));
}

TEST(IteratorsTest, ConstIteratorEmpty) {
    // create empty info object
    mpicxx::info info;

    // begin and end should be the same
    EXPECT_EQ(info.cbegin(), info.cend());

    // create empty const info object
    const mpicxx::info const_info;

    // begin and end should be the same
    EXPECT_EQ(const_info.begin(), const_info.end());
}

TEST(IteratorsDeathTest, NullConstIterator) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling cbegin() or cend() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] mpicxx::info::const_iterator it;
    ASSERT_DEATH( it = info.cbegin() , "");
    ASSERT_DEATH( it = info.cend() , "");

    // create const null info object
    const mpicxx::info const_info(MPI_INFO_NULL, false);

    // calling begin() const or end() const on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( it = const_info.begin() , "");
    ASSERT_DEATH( it = const_info.end() , "");
}