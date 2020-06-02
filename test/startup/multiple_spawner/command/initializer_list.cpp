/**
 * @file test/startup/multiple_spawner/command/initializer_list.cpp
 * @author Marcel Breyer
 * @date 2020-06-02
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::set_command(std::initializer_list<std::string>) member function provided
 *        by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name                                  | test case description                                                                                                 |
 * |:------------------------------------------------|:----------------------------------------------------------------------------------------------------------------------|
 * | SetExecutableNamesViaInitializerList            | set new executable names from a [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) |
 * | SetExecutableNamesViaInitializerListInvalidSize | [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) with illegal size (death test)  |
 * | SetExecutableNamesViaInitializerListInvalidName | try to set new executable names with an invalid name (death test)                                                     |
 */

#include <initializer_list>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>

using namespace std::string_literals;


TEST(MultipleSpawnerTest, SetExecutableNamesViaInitializerList) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names
    ms.set_command({ "baz", "qux" });

    // check if names were set correctly
    ASSERT_EQ(ms.command().size(), 2);
    EXPECT_EQ(ms.command_at(0), "baz"s);
    EXPECT_EQ(ms.command_at(1), "qux"s);
}

TEST(MultipleSpawnerDeathTest, SetExecutableNamesViaInitializerListInvalidSize) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names with different size
    ASSERT_DEATH( ms.set_command({ "baz" }) , "");
    ASSERT_DEATH( ms.set_command({ "baz", "qux", "quux" }) , "");
}

TEST(MultipleSpawnerDeathTest, SetExecutableNamesViaInitializerListInvalidName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ { "foo", 1 }, { "bar", 1 } });

    // set new executable names with illegal name
    ASSERT_DEATH( ms.set_command({ "baz", "" }), "");
}