/**
 * @file empty.cpp
 * @author Marcel Breyer
 * @date 2019-12-16
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `empty` member function of the mpicxx::info class.
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

TEST(CapacityTest, NotEmpty) {
    // create info object and add element
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

TEST(CapacityTest, MovedFromEmpty) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to empty from a "moved-from" info object is invalid
//    [[maybe_unused]] const bool is_empty = info.empty();       // -> should assert
}