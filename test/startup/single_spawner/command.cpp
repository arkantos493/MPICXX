/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::single_spawner::set_command(T&&) and @ref mpicxx::single_spawner::command() const member
 *        functions of the @ref mpicxx::single_spawner class.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name    | test case description                          |
 * |:------------------|:-----------------------------------------------|
 * | SetCommand        | set a new executable name                      |
 * | SetInvalidCommand | set a new illegal executable name (death test) |
 * | GetCommand        | get the current executable name                |
 */

#include <mpicxx/startup/single_spawner.hpp>

#include <gtest/gtest.h>

#include <string>

using namespace std::string_literals;

TEST(SingleSpawnerTest, SetCommand) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    ASSERT_EQ(ss.command(), "a.out"s);

    // set a new executable name
    ss.set_command("b.out");

    // check whether the executable name has been updated
    EXPECT_EQ(ss.command(), "b.out");
}

TEST(SingleSpawnerDeathTest, SetInvalidCommand) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    ASSERT_EQ(ss.command(), "a.out"s);

    // set a new illegal executable name
    ASSERT_DEATH( ss.set_command("") , "");
}

TEST(SingleSpawnerTest, GetCommand) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check getter
    EXPECT_EQ(ss.command(), "a.out"s);
}