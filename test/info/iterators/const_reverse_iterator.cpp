/**
 * @file info/iterators/const_iterator.cpp
 * @author Marcel Breyer
 * @date 2020-02-03
 *
 * @brief Test cases for the @ref mpicxx::info::rbegin() const, @ref mpicxx::info::rend() const, @ref mpicxx::info::crbegin() const and
 * @ref mpicxx::info::crend() const member functions provided by the @ref mpicxx::info class.
 * @details Testsuite: *IteratorsTest*
 * | test case name                | test case description                                         |
 * |:------------------------------|:--------------------------------------------------------------|
 * | ConstReverseIterator          | check for the correct iterator types                          |
 * | ConstReverseIteratorEmpty     | check whether `crbegin() == crend()` for an empty info object |
 * | MovedFromConstReverseIterator | info object in the moved-from state (death test)              |
 */

#include <type_traits>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


template <typename T, typename U>
constexpr bool check_iterator_type(U) { return std::is_same_v<T, U>; }

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

TEST(IteratorsDeathTest, MovedFromConstReverseIterator) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling crbegin() or crend() on an info object in the moved-from state is illegal
    [[maybe_unused]] mpicxx::info::const_reverse_iterator it;
    ASSERT_DEATH( it = info.crbegin() , "");
    ASSERT_DEATH( it = info.crend() , "");

    // create const info object and set it to the moved-from state
    const mpicxx::info const_info(MPI_INFO_NULL, false);

    // calling rbegin() const or rend() const on an info object in the moved-from state is illegal
    ASSERT_DEATH( it = const_info.rbegin() , "");
    ASSERT_DEATH( it = const_info.rend() , "");
}