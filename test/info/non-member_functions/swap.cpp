/**
 * @file test/info/non-member_functions/swap.cpp
 * @author Marcel Breyer
 * @date 2020-04-10
 *
 * @brief Test cases for the @ref mpicxx::info::swap(info&, info&) function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name            | test case description                                              |
 * |:--------------------------|:-------------------------------------------------------------------|
 * | SwapValidAndValid         | swap two info objects                                              |
 * | SwapValidAndMovedFrom     | swap two info objects where one of them is in the moved-from state |
 * | SwapMovedFromAndMovedFrom | swap two info objects where both are in the moved-from state       |
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

TEST(NonMemberFunctionTest, SwapValidAndMovedFrom) {
    // create two info objects and add [key, value]-pairs to one and set the other to the moved-from state
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key", "value");
    mpicxx::info info_2(std::move(info_1));

    // swap both info objects
    using std::swap;
    swap(info_1, info_2);

    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];

    // check info_2 -> now in the default-initialized state
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(info_2.freeable());

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

    // check info_1 -> now in the default-initialized state
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(info_1.freeable());

    // check info_2
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_2.get(), "key", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");
    EXPECT_TRUE(info_2.freeable());
}

TEST(NonMemberFunctionTest, SwapMovedFromAndMovedFrom) {
    // create two info objects and set them to the moved-from state
    mpicxx::info moved_from_1;
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info moved_from_2;
    mpicxx::info dummy_2(std::move(moved_from_2));

    // swap both moved-from info objects
    using std::swap;
    swap(moved_from_1, moved_from_2);

    // both are still in the default-initialized state
    int nkeys_1, nkeys_2;
    MPI_Info_get_nkeys(moved_from_1.get(), &nkeys_1);
    MPI_Info_get_nkeys(moved_from_2.get(), &nkeys_2);
    EXPECT_EQ(nkeys_1, nkeys_2);
    EXPECT_EQ(moved_from_1.freeable(), moved_from_2.freeable());
}