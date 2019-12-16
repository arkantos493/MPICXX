/**
 * @file copy_assignment.cpp
 * @author Marcel Breyer
 * @date 2019-12-16
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the copy assignment operator of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(CopyAssignmentTest, AssignValidToValid) {
    // create first info object
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key1", "value1");
    // create second info object
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key2", "value2");

    // perform assignment
    info_1 = info_2;

    // info_1 should now only contain ["key2", "value2"]
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_1.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    // info_2 should not have changed and only contain ["key2", "value2"]
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_2.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    // be sure that info_1 is really a deep-copy
    MPI_Info_set(info_1.get(), "key3", "value3");
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
}

TEST(CopyAssignmentTest, AssignMovedFromToValid) {
    // create empty info objects
    mpicxx::info moved_from;
    mpicxx::info valid(std::move(moved_from));

    // perform assignment
//    valid = moved_from;     // -> should assert
}

TEST(CopyAssignmentTest, AssignValidToMovedFrom) {
    // create first info object and set it to the "moved-from" state
    mpicxx::info info_1;
    mpicxx::info dummy(std::move(info_1));
    // create second info object
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key2", "value2");

    // perform assignment
    info_1 = info_2;

    // info_1 should not be in the "moved-from" state now
    ASSERT_NE(info_1.get(), MPI_INFO_NULL);

    // info_1 should now only contain ["key2", "value2"]
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_1.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    // info_2 should not have changed and only contain ["key2", "value2"]
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_2.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    // be sure that info_1 is really a deep-copy
    MPI_Info_set(info_1.get(), "key3", "value3");
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
}

TEST(CopyAssignmentTest, AssignMovedFromToMovedFrom) {
    // create empty info objects and set them to the "moved-from" state
    mpicxx::info moved_from_1;
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info moved_from_2;
    mpicxx::info dummy_2(std::move(moved_from_2));

    // perform assignment:
//    moved_from_1 = moved_from_2;     // -> should assert
}

TEST(CopyAssignmentTest, SelfAssignment) {
    // create empty info object
    mpicxx::info info;
    // add a [key, value]-pair
    MPI_Info_set(info.get(), "key", "value");
    const bool is_freeable = info.freeable();

    // perform self-assignment
    info = info;

    // should have done nothing, i.e. its state should not have changed
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    // check number of [key, value]-pairs
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    // check if associated value is correct
    MPI_Info_get(info.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");
    // check freeable state
    EXPECT_EQ(info.freeable(), is_freeable);
}

TEST(CopyAssignmentTest, NonFreeable) {
    // create empty info object
    mpicxx::info info;
    // create non-freable info object
    mpicxx::info non_freeable(MPI_INFO_ENV, false);

    // perform assignment
    non_freeable = info;

    // non_freeable should now be freeable and empty
    int nkeys;
    MPI_Info_get_nkeys(non_freeable.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(non_freeable.freeable());

    // -> if non_freeable would have been freed, the MPI runtime would crash
}