/**
 * @file mpi_info_construction.cpp
 * @author Marcel Breyer
 * @date 2020-01-24
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the constructor of the mpicxx::info class getting a MPI_Info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ConstructionTest, MPIConstructFromFreeableObject) {
    MPI_Info info_ptr;
    MPI_Info_create(&info_ptr);
    MPI_Info_set(info_ptr, "key", "value");

    // construct an info object using a MPI_Info object
    {
        mpicxx::info info(info_ptr, true);

        // info object should now contain 1 entry
        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);
        EXPECT_EQ(nkeys, 1);

        // check if all [key, value]-pairs were added
        int flag;
        char value[MPI_MAX_INFO_VAL];
        MPI_Info_get(info.get(), "key", 5, value, &flag);
        // check if the key exists
        EXPECT_TRUE(static_cast<bool>(flag));
        // be sure that, if the same key is provided multiple times, the last value is used
        EXPECT_STREQ(value, "value");

        // should be the same as the parameter of the constructor
        EXPECT_TRUE(info.freeable());
    }

    // no call to MPI_Info_free necessary!
//    MPI_Info_free(&info_ptr);
}

TEST(ConstructionTest, MPIConstructFromNonFreeableObject) {
    MPI_Info info_ptr;
    MPI_Info_create(&info_ptr);
    MPI_Info_set(info_ptr, "key", "value");

    // construct an info object using a MPI_Info object
    {
        mpicxx::info info(info_ptr, false);

        // info object should now contain 1 entry
        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);
        EXPECT_EQ(nkeys, 1);

        // check if all [key, value]-pairs were added
        int flag;
        char value[MPI_MAX_INFO_VAL];
        MPI_Info_get(info.get(), "key", 5, value, &flag);
        // check if the key exists
        EXPECT_TRUE(static_cast<bool>(flag));
        // be sure that, if the same key is provided multiple times, the last value is used
        EXPECT_STREQ(value, "value");

        // should be the same as the parameter of the constructor
        EXPECT_FALSE(info.freeable());
    }

    // call to MPI_Info_free necessary!
    MPI_Info_free(&info_ptr);
}

TEST(ConstructionTest, MPIConstructFromInvalidObject) {
    [[maybe_unused]] MPI_Info info_ptr = MPI_INFO_NULL;

    // construct an info object using a MPI_Info object
    {
//        [[maybe_unused]] mpicxx::info info(info_ptr, true); // should assert
    }

    info_ptr = MPI_INFO_ENV;

    // construct an info object using a MPI_Info object
    {
//        [[maybe_unused]] mpicxx::info info(info_ptr, true); // should assert
    }
}