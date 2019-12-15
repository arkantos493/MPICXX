/**
 * @file copy_construction.cpp
 * @author Marcel Breyer
 * @date 2019-12-15
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the copy constructor of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(CopyConstructionTest, CreateFromValidObject) {
    mpicxx::info info;
    // add an element to the info object and be sure the key was successfully added
    MPI_Info_set(info.get(), "key", "value");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // save the freeable state of info
    const bool is_freeable = info.freeable();


    // create an new info object by invoking the copy constructor
    mpicxx::info info_copy(info);

    // check if the info_copy object has exactly one element too
    MPI_Info_get_nkeys(info_copy.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // be sure that the moved key and value are present
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info_copy.get(), "key", 5, value, &flag);
    // check if the key exists
    EXPECT_TRUE(static_cast<bool>(flag));
    // check if the value associated with the key is correct
    EXPECT_STREQ(value, "value");

    // add an element to the info object
    MPI_Info_set(info_copy.get(), "key2", "value2");

    // be sure the key was successfully added
    MPI_Info_get_nkeys(info_copy.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // be sure that info_moved has the same freeable state as the "copied-from" object
    EXPECT_EQ(info_copy.freeable(), is_freeable);


    // be sure the copied from object has not been changed
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    MPI_Info_get(info.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");

    EXPECT_EQ(info.freeable(), is_freeable);
}

TEST(CopyConstructionTest, CreateFromMovedFromObject) {
    // create an new info object by invoking the move constructor
    mpicxx::info move_dummy;
    mpicxx::info dummy(std::move(move_dummy));
    // info is now in the "moved-from" state
//    mpicxx::info info_copy(move_dummy);       // -> should assert
}