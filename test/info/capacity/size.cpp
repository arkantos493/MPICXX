/**
 * @file size.cpp
 * @author Marcel Breyer
 * @date 2019-12-16
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `size` member function of the mpicxx::info class.
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
    // create info object and add element
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

TEST(CapacityTest, MovedFromSize) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to empty from a "moved-from" info object is invalid
//    [[maybe_unused]] const mpicxx::info::size_type size = info.size();       // -> should assert
}