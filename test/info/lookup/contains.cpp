/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::contains(const std::string_view) const member function provided by the
 *        @ref mpicxx::info class.
 * @details Testsuite: *LookupTest*
 * | test case name         | test case description                                                                                                    |
 * |:-----------------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | ContainsExisting       | check for existing keys                                                                                                  |
 * | ContainsNonExisting    | check for non-existing key                                                                                               |
 * | NullContains           | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 * | ContainsWithIllegalKey | try to check for the existence of an illegal key (death test)                                                            |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <string>

TEST(LookupTest, ContainsExisting) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // check for the existence of the keys
    EXPECT_TRUE(info.contains("key1"));
    EXPECT_TRUE(info.contains("key2"));
}

TEST(LookupTest, ContainsNonExisting) {
    // create info object and add [key, value]-pair
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // check for the existence of the key
    EXPECT_FALSE(info.contains("key2"));
}

TEST(LookupDeathTest, NullContains) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling contains() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] bool contains;
    ASSERT_DEATH( contains = info.contains("key") , "");
}

TEST(LookupDeathTest, ContainsWithIllegalKey) {
    // create info object
    mpicxx::info info;
    std::string key(MPI_MAX_INFO_KEY, ' ');

    // try to check for the existence of an illegal key
    [[maybe_unused]] bool contains;
    ASSERT_DEATH( contains = info.contains(key) , "");
    ASSERT_DEATH( contains = info.contains("") , "");
}