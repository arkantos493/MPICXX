/**
 * @file info_iterator_test.cpp
 * @author Marcel Breyer
 * @date 2019-11-26
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the iterators of a mpicxx::info object.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoTests, Iterator) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // correct begin() and end() iterators
    EXPECT_TRUE(info.begin() + nkeys == info.end());

    // check all [key, value]-pairs using a range based for loop
    std::string keys = "";
    std::string values = "";
    for (const auto& [key, value]: info) {
        keys += key;
        values += value;
    }
    EXPECT_STREQ(keys.c_str(), "key1key2key3key4");
    EXPECT_STREQ(values.c_str(), "value1value2value3value4");

    // check all [key, value]-pairs using a iterator for loop
    keys = "";
    values = "";
    for (auto it = info.begin(), end = info.end(); it != end; ++it) {
        keys += (*it).first;
        values += (*it).second;
    }
    EXPECT_STREQ(keys.c_str(), "key1key2key3key4");
    EXPECT_STREQ(values.c_str(), "value1value2value3value4");

    // check all [key, value]-pairs using a reversed iterator for loop
    keys = "";
    values = "";
    for (auto it = info.rbegin(), end = info.rend(); it != end; ++it) {
        auto pair = *it;
        keys += pair.first;
        values += pair.second;
    }
    EXPECT_STREQ(keys.c_str(), "key4key3key2key1");
    EXPECT_STREQ(values.c_str(), "value4value3value2value1");

    EXPECT_TRUE(info.begin() == info.begin());
    EXPECT_FALSE(info.begin() == info.end());
    EXPECT_TRUE(info.begin() < info.begin() + 1);
    mpicxx::info info_2;
    EXPECT_FALSE(info.begin() == info_2.begin());

//    mpicxx::info::iterator it = info.begin();
//    mpicxx::info::iterator it_copy = it;
//    mpicxx::info::const_iterator const_it = info.cbegin();
//    mpicxx::info::const_iterator const_it_copy = const_it;
//
//    mpicxx::info::const_iterator const_it_2 = info.begin();
//    mpicxx::info::const_iterator const_it_2_copy = it;
//
//    mpicxx::info::reverse_iterator reverse_it = info.rbegin();
//    mpicxx::info::const_reverse_iterator const_reverse_it = info.crbegin();
//    mpicxx::info::const_reverse_iterator const_reverse_it_2 = info.rbegin();
//    mpicxx::info::const_reverse_iterator const_reverse_it_3 = reverse_it;

//    mpicxx::info::iterator it_3 = info.rbegin();
//    mpicxx::info::reverse_iterator rit = info.begin();
//    mpicxx::info::iterator it_4 = info.crbegin();

}