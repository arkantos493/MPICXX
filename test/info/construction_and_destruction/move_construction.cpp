/**
 * @file move_construction.cpp
 * @author Marcel Breyer
 * @date 2019-12-18
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the move constructor of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ConstructionTest, MoveConstructFromValidObject) {
    // add an element to the info object and be sure the key was successfully added
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // save the freeable state of info
    const bool is_freeable = info.freeable();

    // create an new info object by invoking the move constructor
    mpicxx::info info_move(std::move(info));

    // check if the info_move object has exactly one element too
    MPI_Info_get_nkeys(info_move.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // be sure that the moved key and value are present
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info_move.get(), "key", 5, value, &flag);
    // check if the key exists
    EXPECT_TRUE(static_cast<bool>(flag));
    // check if the value associated with the key is correct
    EXPECT_STREQ(value, "value");

    // add an element to the info object
    MPI_Info_set(info_move.get(), "key2", "value2");

    // be sure the key was successfully added
    MPI_Info_get_nkeys(info_move.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // be sure that info_moved has the same freeable state as the "moved-from" object
    EXPECT_EQ(info_move.freeable(), is_freeable);

    // be sure the moved from object has released it's resources and is now in the "moved-from" state
    EXPECT_EQ(info.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info.freeable());
}

TEST(ConstructionTest, MoveConstructFromMovedFromObject) {
    // add an element to the info object and be sure the key was successfully added
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // create an new info object by invoking the move constructor
    mpicxx::info dummy(std::move(info));
    // info is now in the "moved-from" state
    mpicxx::info info_move(std::move(info));

    // check if the info_move object is also in the "moved-from" state
    EXPECT_EQ(info_move.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_move.freeable());

    // be sure the moved from object has released it's resources and is now in the "moved-from" state
    EXPECT_EQ(info.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info.freeable());
}