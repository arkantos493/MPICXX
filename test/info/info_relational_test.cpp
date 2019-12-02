/**
 * @file info_relational_test.cpp
 * @author Marcel Breyer
 * @date 2019-12-02
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the relational operators of a mpicxx::info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoTests, Equality) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // create copy -> should compare equal
    mpicxx::info info_copy(info);
    EXPECT_TRUE(info == info_copy);

    // change one key -> shouldn't compare equal anymore
    MPI_Info_set(info_copy.get(), "key4", "value4_override");
    EXPECT_FALSE(info == info_copy);

    // remove one key -> shouldn't compare equal
    MPI_Info_delete(info_copy.get(), "key4");
    EXPECT_FALSE(info == info_copy);

}

TEST(InfoTests, Inequality) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // create copy -> shouldn't compare inequal
    mpicxx::info info_copy(info);
    EXPECT_FALSE(info != info_copy);

    // change one key -> should compare inequal
    MPI_Info_set(info_copy.get(), "key4", "value4_override");
    EXPECT_TRUE(info != info_copy);

    // remove one key -> should compare inequal
    MPI_Info_delete(info_copy.get(), "key4");
    EXPECT_TRUE(info != info_copy);

}