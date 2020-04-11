/**
 * @file test/info/modifier/merge.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::merge(info&) function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name           | test case description                                         |
 * |:-------------------------|:--------------------------------------------------------------|
 * | MergeNonEmptyAndNonEmpty | merge two info objects                                        |
 * | MergeEmptyAndEmpty       | merge two info objects (where `*this` and `source` are empty) |
 * | MergeNonEmptyAndEmpty    | merge two info objects (where `source` is empty)              |
 * | MergeEmptyAndNonEmpty    | merge two info objects (where `*this` is empty)               |
 * | NullMerge                | info object referring to MPI_INFO_NULL (death test)           |
 * | SelfMerge                | perform merge with itself (death test)                        |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ModifierTest, MergeNonEmptyAndNonEmpty) {
    // create info objects
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key1", "value1");
    MPI_Info_set(info_1.get(), "key2", "value2");

    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1_override");
    MPI_Info_set(info_2.get(), "key3", "value3");

    // merge info objects
    info_1.merge(info_2);

    // info_1 should have three [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    ASSERT_EQ(nkeys, 3);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info_1.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info_1.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");
    MPI_Info_get(info_1.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // info_2 should have one [key, value]-pair
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    ASSERT_EQ(nkeys, 1);

    MPI_Info_get(info_2.get(), "key1", 15, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1_override");
}

TEST(ModifierTest, MergeEmptyAndEmpty) {
    // create empty info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // merge info objects
    info_1.merge(info_2);

    // info_1 and info_2 should be empty
    int nkeys;
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
}

TEST(ModifierTest, MergeNonEmptyAndEmpty) {
    // create info objects
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key1", "value1");
    MPI_Info_set(info_1.get(), "key2", "value2");

    mpicxx::info info_2;

    // merge info objects
    info_1.merge(info_2);

    // info_1 shouldn't have changed
    int nkeys;
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    ASSERT_EQ(nkeys, 2);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info_1.get(), "key1", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1");
    MPI_Info_get(info_1.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    // info_2 should be empty
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
}

TEST(ModifierTest, MergeEmptyAndNonEmpty) {
    // create info objects
    mpicxx::info info_1;

    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1_override");
    MPI_Info_set(info_2.get(), "key3", "value3");

    // merge info objects
    info_1.merge(info_2);

    // info_1 should now contain two [key, value]-pairs
    int nkeys;
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    ASSERT_EQ(nkeys, 2);

    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info_1.get(), "key1", 15, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value1_override");
    MPI_Info_get(info_1.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // info_2 should now be empty
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
}

TEST(ModifierDeathTest, NullMerge) {
    // create null info objects
    mpicxx::info valid;
    mpicxx::info info_null_1(MPI_INFO_NULL, false);
    mpicxx::info info_null_2(MPI_INFO_NULL, false);

    // calling merge() on/with an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info_null_1.merge(valid) , "");
    ASSERT_DEATH( valid.merge(info_null_2) , "");
    ASSERT_DEATH( info_null_1.merge(info_null_2) , "");
}

TEST(ModifierDeathTest, SelfMerge) {
    // create info object
    mpicxx::info info;

    // perform "self merge"
    ASSERT_DEATH( info.merge(info) , "");
}