/**
 * @file info_lookup_test.cpp
 * @author Marcel Breyer
 * @date 2019-12-02
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the lookup methods of a mpicxx::info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoTests, Find) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"very_long_key1", "value1"},
                          {"k2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // find key3
    mpicxx::info::iterator it = info.find("key3");
    EXPECT_TRUE(it == (info.begin() + 2));

    // change value through iterator
    it->second = "value3_override";

    // check if change was successful
    int flag;
    char value[16];
    MPI_Info_get(info.get(), "key3", 15, value, &flag);
    EXPECT_TRUE(static_cast<bool>(flag));
    EXPECT_STREQ(value, "value3_override");

    // find key 5 -> doesn't exist
    const mpicxx::info const_info(info);
    mpicxx::info::const_iterator const_it = const_info.find("key5");
    EXPECT_TRUE(const_it == const_info.cend());

}

TEST(InfoTests, Count) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"very_long_key1", "value1"},
                          {"k2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // find key3
    EXPECT_EQ(info.count("key3"), 1);

    // find key 5 -> doesn't exist
    EXPECT_EQ(info.count("key5"), 0);

}

TEST(InfoTests, Contains) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"very_long_key1", "value1"},
                          {"k2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // find key3
    EXPECT_TRUE(info.contains("key3"));

    // find key 5 -> doesn't exist
    EXPECT_FALSE(info.contains("key5"));

}