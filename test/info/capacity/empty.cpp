/**
 * @file info/capacity/empty.cpp
 * @author Marcel Breyer
 * @date 2020-01-29
 *
 * @brief Test cases for the @ref mpicxx::info::empty() const member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *CapacityTest*
 * | test case name | test case description                            |
 * |:---------------|:-------------------------------------------------|
 * | Empty          | empty info object                                |
 * | NonEmpty       | non empty info object                            |
 * | MovedFromEmpty | info object in the moved-from state (death test) |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(CapacityTest, Empty) {
    // create empty info object
    mpicxx::info info;

    // info object is empty
    EXPECT_TRUE(info.empty());
}

TEST(CapacityTest, NonEmpty) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // info object is not empty
    EXPECT_FALSE(info.empty());

    // add second element
    MPI_Info_set(info.get(), "key2", "value2");

    // info object is still not empty
    EXPECT_FALSE(info.empty());

    // delete both elements
    MPI_Info_delete(info.get(), "key1");
    MPI_Info_delete(info.get(), "key2");

    // info object is now empty
    EXPECT_TRUE(info.empty());
}

TEST(CapacityDeathTest, MovedFromEmpty) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling empty() on an info object in the moved-from state is illegal
    ASSERT_DEATH(info.empty(), "");
}