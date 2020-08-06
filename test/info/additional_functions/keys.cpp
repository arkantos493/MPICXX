/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-04
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::keys() const member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name | test case description                                                                                                    |
 * |:---------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | NoKeys         | empty info object                                                                                                        |
 * | Keys           | info object with [key, value]-pairs                                                                                      |
 * | NullKeys       | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <vector>

TEST(NonMemberFunctionTest, NoKeys) {
    // create empty info object
    mpicxx::info info;

    // vector of keys should be empty
    std::vector<std::string> keys = info.keys();
    EXPECT_TRUE(keys.empty());
}

TEST(NonMemberFunctionTest, Keys) {
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

    // create vector containing all keys (to compare against)
    std::vector<std::string> correct_keys = { "key1", "key2", "key3", "key4" };

    // get all keys in the info object
    std::vector<std::string> keys = info.keys();

    // compare keys
    ASSERT_EQ(keys.size(), correct_keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(keys[i], correct_keys[i]);
    }
}

TEST(NonMemberFunctionDeathTest, NullKeys) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling key() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] std::vector<mpicxx::info::key_type> res;
    ASSERT_DEATH( res = info.keys() , "");
}