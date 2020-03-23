/**
 * @file test/startup/spawn.cpp
 * @author Marcel Breyer
 * @date 2020-03-23
 *
 * @brief Test cases for the ??? function.
 * @details Testsuite: *StartupTest*
 * | test case name  | test case description                               |
 * |:----------------|:----------------------------------------------------|
 * | Command         | check whether `command` is set correctly            |
 * | EmptyCommand    | create spawner with empty `command` (death test)    |
 * | Maxprocs        | check whether `maxprocs` is set correctly           |
 * | IllegalMaxprocs | create spawner with illegal `maxprocs` (death test) |
 */

#include <limits>
#include <string>

#include <gtest/gtest.h>

#include <mpicxx/startup/spawn.hpp>

using namespace std::string_literals;

TEST(StartupTest, Command) {
    // create spawner
    mpicxx::spawner sp("a.out", 4);

    EXPECT_EQ(sp.command(), "a.out"s);
}

TEST(StartupDeathTest, EmptyCommand) {
    // create spawner with empty command
    ASSERT_DEATH( mpicxx::spawner("", 2) , "");
}

TEST(StartupTest, Maxprocs) {
    // create spawner
    mpicxx::spawner sp("a.out", 4);

    EXPECT_EQ(sp.maxprocs(), 4);
}

TEST(StartupDeathTest, IllegalMaxprocs) {
    // create spawner with illegal maxprocs values
    ASSERT_DEATH( mpicxx::spawner("a.out", -1) , "");
    ASSERT_DEATH( mpicxx::spawner("a.out", std::numeric_limits<int>::max()) , "");
}
