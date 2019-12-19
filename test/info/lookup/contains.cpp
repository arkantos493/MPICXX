/**
 * @file contains.cpp
 * @author Marcel Breyer
 * @date 2019-12-19
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `contains` member function of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(LookupTest, ContainsExisting) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // try finding the keys
    EXPECT_TRUE(info.contains("key1"));
    EXPECT_TRUE(info.contains("key2"));
}

TEST(LookupTest, ContainsNonExisting) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // try finding non-existing key
    EXPECT_FALSE(info.contains("key2"));
}

TEST(LookupTest, MovedFromContains) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to empty from a "moved-from" info object is invalid
//    [[maybe_unused]] const bool contains = info.contains("key");       // -> should assert
}