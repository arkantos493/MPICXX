/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::single_spawner::set_root(int) and @ref mpicxx::single_spawner::root() const member functions
 *        provided by the @ref mpicxx::single_spawner class.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name | test case description                       |
 * |:---------------|:--------------------------------------------|
 * | SetRoot        | set a new root process                      |
 * | SetInvalidRoot | set a new illegal root process (death test) |
 * | GetRoot        | get the current root process                |
 */

#include <mpicxx/startup/single_spawner.hpp>

#include <gtest/gtest.h>

#include <string>

using namespace std::string_literals;

TEST(SingleSpawnerTest, SetRoot) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // set a new root
    ss.set_root(1);

    // check whether the root has been updated
    EXPECT_EQ(ss.root(), 1);
}

TEST(SingleSpawnerDeathTest, SetInvalidRoot) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // set a new illegal root
    ASSERT_DEATH( ss.set_root(-1) , "");
    ASSERT_DEATH( ss.set_root(2)  , "");
}

TEST(SingleSpawnerTest, GetRoot) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check getter
    EXPECT_EQ(ss.root(), 0);
    ss.set_root(1);
    EXPECT_EQ(ss.root(), 1);
}