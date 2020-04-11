/**
 * @file test/info/capacity/size.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::size() const member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *CapacityTest*
 * | test case name | test case description                               |
 * |:---------------|:----------------------------------------------------|
 * | SizeZero       | empty info object                                   |
 * | SizeNonZero    | non empty info object                               |
 * | NullSize       | info object referring to MPI_INFO_NULL (death test) |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(CapacityTest, SizeZero) {
    // create empty info object
    mpicxx::info info;

    // info object is empty -> size is 0
    EXPECT_EQ(info.size(), 0);
}

TEST(CapacityTest, SizeNonZero) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // info object has size 1
    EXPECT_EQ(info.size(), 1);

    // add second element
    MPI_Info_set(info.get(), "key2", "value2");

    // info object now has size 2
    EXPECT_EQ(info.size(), 2);

    // delete both elements
    MPI_Info_delete(info.get(), "key1");
    MPI_Info_delete(info.get(), "key2");

    // info object is now empty
    EXPECT_EQ(info.size(), 0);
}

TEST(CapacityDeathTest, NullSize) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling size() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] mpicxx::info::size_type size;
    ASSERT_DEATH( size = info.size() , "");
}