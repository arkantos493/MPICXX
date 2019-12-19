/**
 * @file count.cpp
 * @author Marcel Breyer
 * @date 2019-12-19
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `count` member function of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(LookupTest, CountExisting) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // try finding the keys
    EXPECT_EQ(info.count("key1"), 1);
    EXPECT_EQ(info.count("key2"), 1);
}

TEST(LookupTest, CountNonExisting) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // try finding non-existing key
    EXPECT_EQ(info.count("key2"), 0);
}

TEST(LookupTest, MovedFromCount) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to empty from a "moved-from" info object is invalid
//    [[maybe_unused]] const int count = info.count("key");       // -> should assert
}