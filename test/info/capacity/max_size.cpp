/**
 * @file max_size.cpp
 * @author Marcel Breyer
 * @date 2020-01-13
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `max_size` member function of the mpicxx::info class.
 */

#include <limits>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(CapacityTest, MaxSize) {
    // create empty info object
    mpicxx::info info;

    // get the maximum possible number of [key, value]-pairs
    EXPECT_EQ(info.max_size(), std::numeric_limits<typename mpicxx::info::difference_type>::max());
}

TEST(CapacityTest, MovedMaxSize) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to max_size from a "moved-from" info object is valid
    EXPECT_EQ(info.max_size(), std::numeric_limits<typename mpicxx::info::difference_type>::max());
}