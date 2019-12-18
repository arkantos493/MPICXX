/**
 * @file initializer_list_assignment.cpp
 * @author Marcel Breyer
 * @date 2019-12-18
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the initializer_list assignment operator of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(AssignmentTest, AssignInitializerListToValid) {
    // create info object and add one element
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // assign initializer_list
    info = { { "key1", "value1" }, { "key2", "value2" } };

    // check if the info object now contains the correct entries
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // old [key, value]-pair should not be present anymore
    MPI_Info_get(info.get(), "key", 5, value, &flag);
    EXPECT_FALSE(static_cast<bool>(flag));

    // ney [key, value]-pairs should be present now
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
}

TEST(AssignmentTest, AssignInitializerListToMovedFrom) {
    // create info object and set it to the "moved-from" state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // assign initializer_list
    info = { { "key1", "value1" }, { "key2", "value2" } };

    // check if the info object now contains the correct entries
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // info should not be in the "moved-from" state anymore
    EXPECT_NE(info.get(), MPI_INFO_NULL);

    // ney [key, value]-pairs should be present now
    MPI_Info_get(info.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
}