/**
 * @file test/startup/single_spawner/root.cpp
 * @author Marcel Breyer
 * @date 2020-04-13
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class root member functions.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name | test case description                                  |
 * |:---------------|:-------------------------------------------------------|
 * | SetRoot        | set a new root process                                 |
 * | SetInvalidRoot | set a new illegal root process (death test)            |
 * | ChainSetRoot   | chain calls to @ref mpicxx::single_spawner::set_root() |
 * | GetRoot        | get the current root process                           |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/single_spawner.hpp>

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

TEST(SingleSpawnerTest, ChainSetRoot) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // chain multiple calls to set_root
    ss.set_root(1).set_root(0).set_root(1);

    // check whether the root has been updated to the last root
    EXPECT_EQ(ss.root(), 1);
}

TEST(SingleSpawnerTest, GetRoot) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check getter
    EXPECT_EQ(ss.root(), 0);
    ss.set_root(1);
    EXPECT_EQ(ss.root(), 1);
}