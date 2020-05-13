/**
 * @file test/startup/multiple_spawner/command/getter.cpp
 * @author Marcel Breyer
 * @date 2020-05-14
 *
 * @brief Test cases for the @ref mpicxx::multiple_spawner::command() const and
 * qref mpicxx::multiple_spawner::command(const std::sze_t) const member function provided by the @ref mpicxx::multiple_spawner class.
 * @details Testsuite: *MultipleSpawnerTest*
 * | test case name       | test case description    |
 * |:---------------------|:-------------------------|
 * | GetExecutableNames   | get all executable names |
 * | GetIthExecutableName | get i-th executable name |
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <mpicxx/startup/multiple_spawner.hpp>

using namespace std::string_literals;


TEST(MultipleSpawnerTest, GetExecutableNames) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // check getter for correctness
    std::vector<std::string> vec = ms.command();
    EXPECT_EQ(vec[0], "foo"s);
    EXPECT_EQ(vec[1], "bar"s);
}

TEST(MultipleSpawnerTest, GetIthExecutableName) {
    // create new multiple_spawner object
    mpicxx::multiple_spawner ms({ {"foo", 1}, {"bar", 1} });

    // check getter for correctness
    EXPECT_EQ(ms.command(0), "foo"s);
    EXPECT_EQ(ms.command(1), "bar"s);
}