/**
 * @file info_modifier_test.cpp
 * @author Marcel Breyer
 * @date 2019-12-02
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the modifying methods of a mpicxx::info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoTests, Clear) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // clear content
    info.clear();
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // invoking another clear should be fine
    info.clear();
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

}