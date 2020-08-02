/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::operator=(const info&) member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *AssignmentTest*
 * | test case name            | test case description                                                          |
 * |:--------------------------|:-------------------------------------------------------------------------------|
 * | CopyAssignValidToValid    | `info1 = info2;`                                                               |
 * | CopyAssignMultiple        | `info1 = info2 = info3; // test *this return`                                  |
 * | CopyAssignNullToValid     | `info1 = info2; // where info2 refers to MPI_INFO_NULL`                        |
 * | CopyAssignValidToNull     | `info1 = info2; // where info1 refers to MPI_INFO_NULL`                        |
 * | CopyAssignNullToNull      | `info1 = info2; // where info1 and info2 refer to MPI_INFO_NULL`               |
 * | CopySelfAssignment        | `info1 = info1; // no-op` (death test)                                         |
 * | CopyAssignToNonFreeable   | non-freeable info object should be freeable now                                |
 * | CopyAssignFromNonFreeable | info object should be freeable (despite that the copied-from was non-freeable) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(AssignmentTest, CopyAssignValidToValid) {
    // create first info object
    mpicxx::info valid_1;
    MPI_Info_set(valid_1.get(), "key1", "value1");
    // create second info object
    mpicxx::info valid_2;
    MPI_Info_set(valid_2.get(), "key2", "value2");

    // perform copy assignment
    valid_1 = valid_2;

    // info_1 should now only contain ["key2", "value2"]
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(valid_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(valid_1.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    // info_2 should not have changed and only contain ["key2", "value2"]
    MPI_Info_get_nkeys(valid_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(valid_2.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    // be sure that info_1 really is a deep-copy
    // -> add element to info_1 and be sure that info_2 still has only one [key, value]-pair
    MPI_Info_set(valid_1.get(), "key3", "value3");
    MPI_Info_get_nkeys(valid_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
}

TEST(AssignmentTest, CopyAssignMultiple) {
    // create first info object
    mpicxx::info info_1;
    MPI_Info_set(info_1.get(), "key1", "value1");
    // create second info object
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key2", "value2");
    // create third info object
    mpicxx::info info_3;
    MPI_Info_set(info_3.get(), "key3", "value3");

    // perform copy assignment
    info_1 = info_2 = info_3;

    // info_1 should now only contain ["key3", "value3"]
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info_1.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_1.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // info_2 should now only contain ["key3", "value3"] too
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_2.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");
}

TEST(AssignmentTest, CopyAssignNullToValid) {
    // create info object and null info object
    mpicxx::info info_null(MPI_INFO_NULL, false);
    mpicxx::info valid;

    // copy assign moved-from object
    valid = info_null;

    // info_null and valid should refer to MPI_INFO_NULL
    EXPECT_EQ(info_null.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null.freeable());
    EXPECT_EQ(valid.get(), MPI_INFO_NULL);
    EXPECT_FALSE(valid.freeable());
}

TEST(AssignmentTest, CopyAssignValidToNull) {
    // create null info object
    mpicxx::info info_null(MPI_INFO_NULL, false);
    // create second info object
    mpicxx::info valid;
    MPI_Info_set(valid.get(), "key", "value");

    // perform copy assignment
    info_null = valid;

    // moved_from should not be in the moved-from state anymore
    ASSERT_NE(info_null.get(), MPI_INFO_NULL);

    // info_null should now contain ["key2", "value2"]
    int nkeys, flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get_nkeys(info_null.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(info_null.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");

    // valid should not have changed and only contain ["key2", "value2"]
    MPI_Info_get_nkeys(valid.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    MPI_Info_get(valid.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");

    // be sure that info_null really is a deep-copy
    // -> add element to info_null and be sure that valid still has only one [key, value]-pair
    MPI_Info_set(info_null.get(), "key2", "value2");
    MPI_Info_get_nkeys(valid.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
}

TEST(AssignmentTest, CopyAssignNullToNull) {
    // create two null info objects
    mpicxx::info info_null_1(MPI_INFO_NULL, false);
    mpicxx::info info_null_2(MPI_INFO_NULL, false);

    // copy assign moved-from object
    info_null_1 = info_null_2;

    // info_null_1 and info_null_2 should refer to MPI_INFO_NULL
    EXPECT_EQ(info_null_1.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null_1.freeable());
    EXPECT_EQ(info_null_2.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null_2.freeable());
}

TEST(AssignmentDeathTest, CopySelfAssignment) {
    // create info object
    mpicxx::info info;

    // perform self assignment
    EXPECT_DEATH( info = info, "");
}

TEST(AssignmentTest, CopyAssignToNonFreeable) {
    // create empty info object
    mpicxx::info info;
    // create non-freeable info object
    mpicxx::info non_freeable(MPI_INFO_ENV, false);

    // perform copy assignment
    non_freeable = info;

    // non_freeable should now be freeable and empty
    int nkeys;
    MPI_Info_get_nkeys(non_freeable.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    EXPECT_TRUE(non_freeable.freeable());

    // -> if non_freeable would have been freed, the MPI runtime would crash
}

TEST(AssignmentTest, CopyAssignFromNonFreeable) {
    // create empty info object
    mpicxx::info info;
    // create non-freeable info object
    mpicxx::info non_freeable(MPI_INFO_ENV, false);

    // perform copy assignment
    info = non_freeable;

    // info should now have as many keys as info and should be marked freeable
    int nkeys_info, nkeys_non_freeable;
    MPI_Info_get_nkeys(non_freeable.get(), &nkeys_non_freeable);
    MPI_Info_get_nkeys(info.get(), &nkeys_info);
    EXPECT_EQ(nkeys_info, nkeys_non_freeable);
    EXPECT_TRUE(info.freeable());
}