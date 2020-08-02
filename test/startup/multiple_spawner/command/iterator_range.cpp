/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_command(InputIt, InputIt) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                | test case description                                             |
 * |:----------------------------------------------|:------------------------------------------------------------------|
 * | SetExecutableNamesViaIteratorRange            | set new executable names from an iterator range                   |
 * | SetExecutableNamesViaInvalidIteratorRange     | illegal iterator range (death test)                               |
 * | SetExecutableNamesViaIteratorRangeInvalidSize | iterator range with illegal size (death test)                     |
 * | SetExecutableNamesViaIteratorRangeInvalidName | try to set new executable names with an invalid name (death test) |
 */

#include <mpicxx/startup/multiple_spawner.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

TEST(MultipleSpawnerTest, SetExecutableNamesViaIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names
    std::vector<std::string> vec = { "baz", "qux" };
    ms.set_command(vec.begin(), vec.end());

    // check if the names were set correctly
    ASSERT_EQ(ms.command().size(), 2);
    for (std::size_t i = 0; i < 2; ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(ms.command_at(i), vec[i]);
    }
}

TEST(MultipleSpawnerDeathTest, SetExecutableNamesViaInvalidIteratorRange) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names with illegal iterator range
    std::vector<std::string> vec = { "baz", "qux" };
    ASSERT_DEATH( ms.set_command(vec.end(), vec.begin()) , "");
}

TEST(MultipleSpawnerDeathTest, SetExecutableNamesViaIteratorRangeInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names with different size
    std::vector<std::string> vec = { "baz", "qux", "quux" };
    ASSERT_DEATH( ms.set_command(vec.begin(), vec.begin() + 1) , "");
    ASSERT_DEATH( ms.set_command(vec.begin(), vec.end()) , "");
}

TEST(MultipleSpawnerDeathTest, SetExecutableNamesViaIteratorRangeInvalidName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names with illegal name
    std::vector<std::string> vec = { "baz", "" };
    ASSERT_DEATH( ms.set_command(vec.begin(), vec.end()), "");
}