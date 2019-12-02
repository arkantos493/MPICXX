/**
 * @file info_non-member_functions_test.cpp
 * @author Marcel Breyer
 * @date 2019-12-02
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the non-member functions (i.e. relational operators and specializations of std::swap and
 * std::erase_if) of a mpicxx::info object.
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

TEST(InfoTests, Swap) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };
    // construct an empty info object
    mpicxx::info empty_info;

    // check correct size
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);
    MPI_Info_get_nkeys(empty_info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // swap content and check new sizes
    info.swap(empty_info);
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
    MPI_Info_get_nkeys(empty_info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // swap content back and check sizes again
    std::swap(info, empty_info);
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);
    MPI_Info_get_nkeys(empty_info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

}