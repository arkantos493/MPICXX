/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::info(const info&) member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ConstructionTest*
 * | test case name               | test case description                                                          |
 * |:-----------------------------|:-------------------------------------------------------------------------------|
 * | CopyConstructFromValidObject | `mpicxx::info info1(info2);`                                                   |
 * | CopyConstructFromNullObject  | `mpicxx::info info1(info2); // where info2 refers to MPI_INFO_NULL`            |
 * | CopyConstructFromNonFreeable | info object should be freeable (despite that the copied-from was non-freeable) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(ConstructionTest, CopyConstructFromValidObject) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key", "value");

    // save the freeable state of info
    const bool is_freeable = info.freeable();

    // create an new info object by invoking the copy constructor
    mpicxx::info info_copy(info);

    // check if info_copy has exactly one element
    int nkeys;
    MPI_Info_get_nkeys(info_copy.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // be sure that the copied [key, value]-pair ist present
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info_copy.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");

    // be sure that info_copy really is a deep-copy
    // -> add element to info_1 and be sure that info_2 still has only one [key, value]-pair
    MPI_Info_set(info_copy.get(), "key2", "value2");
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // be sure that info_copy is marked freeable
    EXPECT_EQ(info_copy.freeable(), is_freeable);

    // be sure the copied from info object has not been changed
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    MPI_Info_get(info.get(), "key", 5, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value");

    EXPECT_EQ(info.freeable(), is_freeable);
}

TEST(ConstructionTest, CopyConstructFromNullObject) {
    // create null info object
    mpicxx::info info_null(MPI_INFO_NULL, false);

    // create new info object via the copy constructor
    mpicxx::info valid(info_null);

    // info_null should still refer to MPI_INFO_NULL
    EXPECT_EQ(info_null.get(), MPI_INFO_NULL);
    EXPECT_FALSE(info_null.freeable());

    // valid should refer to MPI_INFO_NULL
    EXPECT_EQ(valid.get(), MPI_INFO_NULL);
    EXPECT_FALSE(valid.freeable());
}

TEST(ConstructionTest, CopyConstructFromNonFreeable) {
    // create non-freeable info object
    mpicxx::info non_freeable(MPI_INFO_ENV, false);

    // create an new info object by invoking the copy constructor
    mpicxx::info info = non_freeable;

    // info should now have as many keys as info and should be marked freeable
    int nkeys_info, nkeys_non_freeable;
    MPI_Info_get_nkeys(non_freeable.get(), &nkeys_non_freeable);
    MPI_Info_get_nkeys(info.get(), &nkeys_info);
    EXPECT_EQ(nkeys_info, nkeys_non_freeable);
    EXPECT_TRUE(info.freeable());
}