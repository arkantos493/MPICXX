/**
 * @file values.cpp
 * @author Marcel Breyer
 * @date 2019-12-16
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `values` member function provided for a mpicxx::info object.
 */

#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, NoValues) {
    // create info object and add various keys
    mpicxx::info info;

    // no [key, value]-pairs have been added yet so the vector should be empty
    std::vector<std::string> values = info.values();
    EXPECT_TRUE(values.empty());
}

TEST(NonMemberFunctionTest, Values) {
    // create info object and add various [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");
    MPI_Info_set(info.get(), "key4", "value4");

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // create vector containing all values
    std::vector<std::string> correct_values = { "value1", "value2", "value3", "value4" };

    // get values in this info object
    std::vector<std::string> values = info.values();

    // compare values
    ASSERT_EQ(values.size(), correct_values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(values[i], correct_values[i]);
    }
}