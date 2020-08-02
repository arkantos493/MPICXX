/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::swap(info&) member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name    | test case description                                                                                                                |
 * |:------------------|:-------------------------------------------------------------------------------------------------------------------------------------|
 * | SwapValidAndValid | swap two info objects                                                                                                                |
 * | SwapValidAndNull  | swap two info objects where one of them refers to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) |
 * | SwapNullAndNull   | swap two info objects where both refer to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm)         |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(ModifierTest, SwapValidAndValid) {
    // create two info objects and add [key, value]-pairs
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key1", "value1");
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key2", "value2");
    MPI_Info_set(info_2.get(), "key3", "value3");

    // swap both info objects
    info_1.swap(info_2);

    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    // check info_1 object
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);
    MPI_Info_get(info_1.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
    MPI_Info_get(info_1.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // check info_2 object
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_2.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
}

TEST(ModifierTest, SwapValidAndNull) {
    // create info object and add [key, value]-pairs and another null info object
    mpicxx::info info_1(MPI_INFO_NULL, false);
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key", "value");

    // swap both info objects
    info_1.swap(info_2);

    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];

    // check info_2 -> now referring to MPI_INFO_NULL
    EXPECT_EQ(info_2.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_2.freeable());

    // check info_1
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_1.get(), "key", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");
    EXPECT_TRUE(info_1.freeable());

    // swap both info objects back
    info_1.swap(info_2);

    // check info_1 -> now referring to MPI_INFO_NULL
    EXPECT_EQ(info_1.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_1.freeable());

    // check info_2
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_2.get(), "key", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");
    EXPECT_TRUE(info_2.freeable());
}

TEST(ModifierTest, SwapNullAndNull) {
    // create two null info objects
    mpicxx::info info_null_1(MPI_INFO_NULL, false);
    mpicxx::info info_null_2(MPI_INFO_NULL, false);

    // swap both info objects
    info_null_1.swap(info_null_2);

    // both are still referring to MPI_INFO_NULL
    EXPECT_EQ(info_null_1.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null_1.freeable());
    EXPECT_EQ(info_null_2.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null_2.freeable());
}