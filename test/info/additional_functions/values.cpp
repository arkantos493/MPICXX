/**
 * @file info/additional_functions/values.cpp
 * @author Marcel Breyer
 * @date 2020-01-27
 *
 * @brief Test cases for the @ref mpicxx::info::values() const member function provided by the @ref mpicxx::info class.
 * * @details Testsuite: *NonMemberFunctionTest*
 * | test case name   | test case description                            |
 * |:-----------------|:-------------------------------------------------|
 * | NoValues         | empty info object                                |
 * | Values           | info object with [key, value]-pairs              |
 * | MovedFromValues  | info object in the moved-from state (death test) |
 */

#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, NoValues) {
    // create empty info object
    mpicxx::info info;

// vector of values should be empty
    std::vector<std::string> values = info.values();
    EXPECT_TRUE(values.empty());
}

TEST(NonMemberFunctionTest, Values) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");
    MPI_Info_set(info.get(), "key4", "value4");

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // create vector containing all values (to compare against)
    std::vector<std::string> correct_values = { "value1", "value2", "value3", "value4" };

    // get all values in the info object
    std::vector<std::string> values = info.values();

    // compare values
    ASSERT_EQ(values.size(), correct_values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(values[i], correct_values[i]);
    }
}

TEST(NonMemberFunctionDeathTest, MovedFromValues) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling value() on an info object in the moved-from state is illegal
    ASSERT_DEATH(info.values(), "");
}