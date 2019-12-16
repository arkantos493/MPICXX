/**
 * @file move_assignment.cpp
 * @author Marcel Breyer
 * @date 2019-12-16
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the move assignment operator of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(MoveAssignmentTest, AssignValidToValid) {
    // create empty info objects and add one element each
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key1", "value1");
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key2", "value2");

    // perform assignment
    info_1 = std::move(info_2);

    // info_2 should now be in the "moved-from" state
    EXPECT_EQ(info_2.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_2.freeable());
    // info_1 should be in a valid state containing one [key, value]-pair
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_1.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
}

TEST(MoveAssignmentTest, AssignMovedFromToValid) {
    // create empty info objects and set them to the "moved-from" state
    mpicxx::info moved_from;
    mpicxx::info valid(std::move(moved_from));

    // perform assignment
    valid = std::move(moved_from);

    // now both info objects should be in the "moved-from" state
    EXPECT_EQ(valid.get(), MPI_INFO_NULL);
    EXPECT_FALSE(valid.freeable());
    EXPECT_EQ(moved_from.get(), MPI_INFO_NULL);
    EXPECT_FALSE(moved_from.freeable());
}

TEST(MoveAssignmentTest, AssignValidToMovedFrom) {
    // create empty info objects and set them to the "moved-from" state
    mpicxx::info moved_from;
    mpicxx::info valid(std::move(moved_from));
    MPI_Info_set(valid.get(), "key", "value");

    // perform assignment
    moved_from = std::move(valid);

    // valid should now be in the "moved-from" state
    EXPECT_EQ(valid.get(), MPI_INFO_NULL);
    EXPECT_FALSE(valid.freeable());
    // moved_from should now be in a valid state containing one [key, value]-pair
    ASSERT_NE(moved_from.get(), MPI_INFO_NULL);
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(moved_from.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(moved_from.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");
}

TEST(MoveAssignmentTest, AssignMovedFromToMovedFrom) {
    // create empty info objects and set them to the "moved-from" state
    mpicxx::info moved_from_1;
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info moved_from_2;
    mpicxx::info dummy_2(std::move(moved_from_2));

    // perform assignment
    moved_from_1 = std::move(moved_from_2);

    // now both info objects should still be in the "moved-from" state
    EXPECT_EQ(moved_from_1.get(), MPI_INFO_NULL);
    EXPECT_FALSE(moved_from_1.freeable());
    EXPECT_EQ(moved_from_2.get(), MPI_INFO_NULL);
    EXPECT_FALSE(moved_from_2.freeable());
}

TEST(MoveAssignmentTest, NonFreeable) {
    // create empty info object
    mpicxx::info info;
    // create non-freable info object
    mpicxx::info non_freeable(MPI_INFO_ENV, false);

    // perform assignment
    non_freeable = std::move(info);

    // non_freeable should now be freeable and empty
    int nkeys;
    MPI_Info_get_nkeys(non_freeable.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(non_freeable.freeable());

    // info should be in the "moved-from" state
    EXPECT_EQ(info.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info.freeable());

    // -> if non_freeable would have been freed, the MPI runtime would crash
}