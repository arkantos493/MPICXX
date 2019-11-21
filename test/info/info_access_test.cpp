/**
 * @file info_access_test.cpp
 * @author Marcel Breyer
 * @date 2019-11-20
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the element access functions of a mpicxx::info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoTests, ElementRead) {

    // default construct a mpicxx::info object
    mpicxx::info info;

    // add an element to the info object
    MPI_Info_set(info.get(), "key", "value");
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // read currently added value
    std::string value = info["key"];
    EXPECT_STREQ(value.c_str(), "value");

    // add new [key, value]-pair
    info["key2"] = "value2";
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

}