/**
 * @file info_additional_functions_test.cpp
 * @author Marcel Breyer
 * @date 2019-12-12
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the additional functions provided for a mpicxx::info object.
 */

#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(InfoTests, Keys) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // create vector containing all keys
    std::vector<std::string> correct_keys = { "key1", "key2", "key3", "key4" };

    // get keys in this info object
    std::vector<std::string> keys = info.keys();

    // compare keys
    EXPECT_EQ(keys.size(), correct_keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(keys[i], correct_keys[i]);
    }

}

TEST(InfoTests, Values) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // create vector containing all values
    std::vector<std::string> correct_values = { "value1", "value2", "value3", "value4" };

    // get values in this info object
    std::vector<std::string> values = info.values();

    // compare values
    EXPECT_EQ(values.size(), correct_values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(values[i], correct_values[i]);
    }

}