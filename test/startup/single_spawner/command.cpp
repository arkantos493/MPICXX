/**
 * @file test/startup/single_spawner/command.cpp
 * @author Marcel Breyer
 * @date 2020-04-13
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class command member functions.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name    | test case description                                     |
 * |:------------------|:----------------------------------------------------------|
 * | SetCommand        | set a new command name                                    |
 * | SetCommandInvalid | set a new illegal command name (death test)               |
 * | ChainSetCommand   | chain calls to @ref mpicxx::single_spawner::set_command() |
 * | GetCommand        | get the current command name                              |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/single_spawner.hpp>

using namespace std::string_literals;


TEST(SingleSpawnerTest, SetCommand) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    ASSERT_EQ(ss.command(), "a.out"s);

    // set a new command name
    ss.set_command("b.out");

    // check whether the command name has been updated
    EXPECT_EQ(ss.command(), "b.out");
}

TEST(SingleSpawnerDeathTest, SetCommandInvalid) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    ASSERT_EQ(ss.command(), "a.out"s);

    // set a new illegal command name
    ASSERT_DEATH( ss.set_command("") , "");
}

TEST(SingleSpawnerTest, ChainSetCommand) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    ASSERT_EQ(ss.command(), "a.out"s);

    // chain multiple calls to set_command
    ss.set_command("b.out").set_command("c.out").set_command("d.out");

    // check whether the command name has been updated to the last command name
    EXPECT_EQ(ss.command(), "d.out");
}

TEST(SingleSpawnerTest, GetCommand) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check getter
    EXPECT_EQ(ss.command(), "a.out"s);
}