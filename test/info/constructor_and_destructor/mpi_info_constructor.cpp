/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::info(MPI_Info, const bool) member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ConstructionTest*
 * | test case name                    | test case description                                                                                                                                                                                 |
 * |:----------------------------------|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * | MPIConstructFromFreeableObject    | freeable [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object                                                                                                         |
 * | MPIConstructFromNonFreeableObject | non-freeable MPI_Info object                                                                                                                                                                          |
 * | MPIConstructFromInvalidObject     | [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) and [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) shouldn't be marked freeable |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(ConstructionTest, MPIConstructFromFreeableObject) {
    MPI_Info info_ptr;
    MPI_Info_create(&info_ptr);
    MPI_Info_set(info_ptr, "key", "value");

    // construct an info object using a MPI_Info object
    {
        mpicxx::info info(info_ptr, true);

        // info object should now contain one entry
        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);
        EXPECT_EQ(nkeys, 1);

        // check if the correct [key, value]-pair has been added
        int flag;
        char value[MPI_MAX_INFO_VAL];
        MPI_Info_get(info.get(), "key", 5, value, &flag);
        EXPECT_TRUE(static_cast<bool>(flag));
        EXPECT_STREQ(value, "value");

        // should be freeable
        EXPECT_TRUE(info.freeable());

        // changing the underlying MPI_Info object DOESN'T change the mpicxx::info object
        info_ptr = MPI_INFO_ENV;
        MPI_Info_get_nkeys(info.get(), &nkeys);
        EXPECT_EQ(nkeys, 1);
    }

    // -> no call to MPI_Info_free necessary!
}

TEST(ConstructionTest, MPIConstructFromNonFreeableObject) {
    MPI_Info info_ptr;
    MPI_Info_create(&info_ptr);
    MPI_Info_set(info_ptr, "key", "value");

    // construct an info object using a MPI_Info object
    {
        mpicxx::info info(info_ptr, false);

        // info object should now contain one entry
        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);
        EXPECT_EQ(nkeys, 1);

        // check if the correct [key, value]-pair has been added
        int flag;
        char value[MPI_MAX_INFO_VAL];
        MPI_Info_get(info.get(), "key", 5, value, &flag);
        EXPECT_TRUE(static_cast<bool>(flag));
        EXPECT_STREQ(value, "value");

        // should be false
        EXPECT_FALSE(info.freeable());
    }

    // call to MPI_Info_free necessary!
    MPI_Info_free(&info_ptr);
}

TEST(ConstructionDeathTest, MPIConstructFromInvalidObject) {
    MPI_Info info_null_ptr = MPI_INFO_NULL;
    // construct an info object using a MPI_Info object
    {
        ASSERT_DEATH( mpicxx::info info(info_null_ptr, true) , "");
    }

    MPI_Info info_env_ptr = MPI_INFO_ENV;
    // construct an info object using a MPI_Info object
    {
        ASSERT_DEATH( mpicxx::info info(info_env_ptr, true) , "");
    }
}