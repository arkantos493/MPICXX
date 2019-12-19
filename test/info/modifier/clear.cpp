/**
 * @file clear.cpp
 * @author Marcel Breyer
 * @date 2019-12-19
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `clear` member function of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, Clear) {
    // create empty info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");

    // size should be 4
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 3);

    // try clearing info object
    info.clear();

    // now the size should be 0
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // clearing again should do nothing
    info.clear();
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
}

TEST(ModifierTest, MovedFromClear) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // call to empty from a "moved-from" info object is invalid
//    info.clear();       // -> should assert
}