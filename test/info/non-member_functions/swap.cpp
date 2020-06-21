/**
 * @file test/info/non-member_functions/swap.cpp
 * @author Marcel Breyer
 * @date 2020-04-10
 *
 * @brief Test cases for the @ref mpicxx::info::swap(info&, info&) function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name    | test case description                                             |
 * |:------------------|:------------------------------------------------------------------|
 * | SwapValidAndValid | swap two info objects                                             |
 * | SwapValidAndNull  | swap two info objects where one of them refers to *MPI_INFO_NULL* |
 * | SwapNullAndNull   | swap two info objects where both refer to *MPI_INFO_NULL*         |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, SwapValidAndValid) {
    // create two info objects and add [key, value]-pairs
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key1", "value1");
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key2", "value2");
    MPI_Info_set(info_2.get(), "key3", "value3");

    // swap both info objects
    using std::swap;
    swap(info_1, info_2);

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

TEST(NonMemberFunctionTest, SwapValidAndNull) {
    // create two info objects and add [key, value]-pairs to one and set the other to refer to MPI_INFO_NULL
    mpicxx::info info_1(MPI_INFO_NULL, false);
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key", "value");

    // swap both info objects
    using std::swap;
    swap(info_1, info_2);

    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];

    // check info_2 -> now refers to MPI_INFO_NULL
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
    using std::swap;
    swap(info_1, info_2);

    // check info_1 -> now refers to MPI_INFO_NULL
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

TEST(NonMemberFunctionTest, SwapNullAndNull) {
    // create two null info objects
    mpicxx::info info_null_1(MPI_INFO_NULL, false);
    mpicxx::info info_null_2(MPI_INFO_NULL, false);

    // swap both null info objects
    using std::swap;
    swap(info_null_1, info_null_2);

    // both are still referring to MPI_INFO_NULL
    EXPECT_EQ(info_null_1.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null_1.freeable());
    EXPECT_EQ(info_null_2.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null_2.freeable());
}