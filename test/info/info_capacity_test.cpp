/**
 * @file info_capacity_test.cpp
 * @author Marcel Breyer
 * @date 2019-11-20
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the capacity functions of a mpicxx::info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoTests, CapacitySize) {

    // default construct a mpicxx::info object
    mpicxx::info info;

    // no key added yet
    EXPECT_EQ(info.size(), 0);

    // add an element to the info object
    MPI_Info_set(info.get(), "key", "value");
    EXPECT_EQ(info.size(), 1);

    // add another element to the info object
    MPI_Info_set(info.get(), "key2", "value2");
    EXPECT_EQ(info.size(), 2);

    // remove previously added elements
    MPI_Info_delete(info.get(), "key");
    MPI_Info_delete(info.get(), "key2");
    EXPECT_EQ(info.size(), 0);

}

TEST(InfoTests, CapacityEmpty) {

    // default construct a mpicxx::info object
    mpicxx::info info;

    // no key added yet
    EXPECT_TRUE(info.empty());

    // add an element to the info object
    MPI_Info_set(info.get(), "key", "value");
    EXPECT_FALSE(info.empty());

    // add another element to the info object
    MPI_Info_set(info.get(), "key2", "value2");
    EXPECT_FALSE(info.empty());

    // remove previously added elements
    MPI_Info_delete(info.get(), "key");
    MPI_Info_delete(info.get(), "key2");
    EXPECT_TRUE(info.empty());

}