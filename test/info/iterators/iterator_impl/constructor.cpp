/**
 * @file info/iterators/iterator_impl/constructor.cpp
 * @author Marcel Breyer
 * @date 2020-02-12
 *
 * @brief Test cases for the constructors of the @ref mpicxx::info::iterator and @ref mpicxx::info::const_iterator class.
 * @details Testsuite: *IteratorImplTest*
 * | test case name                 | test case description                                                                  |
 * |:-------------------------------|:---------------------------------------------------------------------------------------|
 * | DefaultConstruct               | default construct a singular iterator (death test)                                     |
 * | ConstructFromInfoObjectValid   | construct an iterator referring to an info object                                      |
 * | ConstructFromInfoObjectInvalid | construct an iterator referring to an info object in the moved-from state (death test) |
 * | CopyConstructValid             | construct an iterator from another, valid iterator                                     |
 * | CopyConstructInvalid           | construct an iterator from another, invalid iterator (death test)                      |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(IteratorImplDeathTest, DefaultConstruct) {
    // default construct iterator
    mpicxx::info::iterator it;

    // calling ANY function on a singular iterator asserts
    EXPECT_DEATH(it++, "");
}


TEST(IteratorImplTest, ConstructFromInfoObjectValid) {
    // create info object
    mpicxx::info info;

    // construct iterator
    mpicxx::info::const_iterator it = mpicxx::info::const_iterator(info.get(), 0);

    EXPECT_TRUE(it == info.begin());
}

TEST(IteratorImplDeathTest, ConstructFromInfoObjectInvalid) {
    // create info object
    mpicxx::info moved_from;
    mpicxx::info info(std::move(moved_from));

    // explicitly construct a singular iterator
    EXPECT_DEATH(mpicxx::info::const_iterator(nullptr, 0), "");

    // construct iterator from info object in the moved-from state
    EXPECT_DEATH(mpicxx::info::iterator(moved_from.get(), 0), "");

    // construct iterator with illegal start positions
    EXPECT_DEATH(mpicxx::info::const_iterator(info.get(), -1), "");
    EXPECT_DEATH(mpicxx::info::const_iterator(info.get(), 1), "");
}


TEST(IteratorImplTest, CopyConstructValid) {
    // create info object
    mpicxx::info info;
    const mpicxx::info const_info;

    // construct iterators
    mpicxx::info::iterator it = info.begin();
    mpicxx::info::const_iterator const_it = info.cbegin();

    // test copy construction
    mpicxx::info::iterator it_copy_1(it);
    EXPECT_TRUE(it_copy_1 == it);
//    mpicxx::info::iterator it_copy_2(const_it);   // shouldn't compile
    mpicxx::info::const_iterator const_it_copy_1(it);
    EXPECT_TRUE(const_it_copy_1 == it);
    mpicxx::info::const_iterator const_it_copy_2(const_it);
    EXPECT_TRUE(const_it_copy_2 == const_it);
}

TEST(IteratorImplTest, CopyConstructInvalid) {
    // create info object
    mpicxx::info moved_from;
    mpicxx::info::iterator moved_from_it = moved_from.begin();
    mpicxx::info info(std::move(moved_from));

    // copy construct from singular iterator
    mpicxx::info::iterator sit;
    EXPECT_DEATH( mpicxx::info::iterator it_1(sit) , "");

    // copy construct iterator from an iterator that refers to an info object in the moved-from state
    EXPECT_DEATH( mpicxx::info::iterator it_2(moved_from_it) , "");
}