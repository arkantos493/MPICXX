/**
 * @file initializer_list_construction.cpp
 * @author Marcel Breyer
 * @date 2019-12-18
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the initializer_list constructor of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(ConstructionTest, InitializerListConstruction) {
    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key1", "value1_override"},
                          {"key3", "value3"} };

    // info object should now contain 3 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 3);

    // check if all [key, value]-pairs were added
    int flag;
    char value[MPI_MAX_INFO_VAL];
    MPI_Info_get(info.get(), "key1", 15, value, &flag);
    // check if the key exists
    EXPECT_TRUE(static_cast<bool>(flag));
    // be sure that, if the same key is provided multiple times, the last value is used
    EXPECT_STREQ(value, "value1_override");

    MPI_Info_get(info.get(), "key2", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value2");

    MPI_Info_get(info.get(), "key3", 6, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3");

    // an info object constructed from an initializer_list is always freeable
    EXPECT_TRUE(info.freeable());
}