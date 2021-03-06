/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_command(T&&...) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                | test case description                                             |
 * |:----------------------------------------------|:------------------------------------------------------------------|
 * | SetExecutableNamesViaParameterPack            | set new executable names from a parameter pack                    |
 * | SetExecutableNamesViaParameterPackInvalidSize | parameter pack with illegal size (death test)                     |
 * | SetExecutableNamesViaParameterPackInvalidName | try to set new executable names with an invalid name (death test) |
 */

#include <mpicxx/startup/multiple_spawner.hpp>

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <utility>

using namespace std::string_literals;

TEST(MultipleSpawnerTest, SetExecutableNamesViaParameterPack) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names
    ms.set_command("baz", "qux"s);

    // check if names were set correctly
    ASSERT_EQ(ms.command().size(), 2);
    EXPECT_EQ(ms.command_at(0), "baz"s);
    EXPECT_EQ(ms.command_at(1), "qux"s);
}

TEST(MultipleSpawnerDeathTest, SetExecutableNamesViaParameterPackInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names with different size
    ASSERT_DEATH( ms.set_command("baz") , "");
    std::string str("quux");
    ASSERT_DEATH( ms.set_command("baz", "qux"s, str) , "");
}

TEST(MultipleSpawnerDeathTest, SetExecutableNamesViaParameterPackInvalidName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names with illegal name
    ASSERT_DEATH( ms.set_command("baz", ""), "");
}