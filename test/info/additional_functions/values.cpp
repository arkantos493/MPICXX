/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::values() const member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name | test case description                                                                                                    |
 * |:---------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | NoValues       | empty info object                                                                                                        |
 * | Values         | info object with [key, value]-pairs                                                                                      |
 * | NullValues     | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <vector>

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

TEST(NonMemberFunctionDeathTest, NullValues) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling value() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.values() , "");
}