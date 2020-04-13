/**
 * @file test/startup/single_spawner/constructor.cpp
 * @author Marcel Breyer
 * @date 2020-04-13
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class constructors.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name                   | test case description                                                                                                       |
 * |:---------------------------------|:----------------------------------------------------------------------------------------------------------------------------|
 * | ConstructValid                   | test the spawning of a new MPI process                                                                                      |
 * | ConstructInvalidCommand          | spawn with an illegal command name (death test)                                                                             |
 * | ConstructInvalidMaxprocs         | spawn with an illegal number of maxprocs (death test)                                                                       |
 * | ConstructFromPairValid           | test the spawning of a new MPI process using a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair)                |
 * | ConstructFromPairInvalidCommand  | spawn with an illegal command name using a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) (death test)       |
 * | ConstructFromPairInvalidMaxprocs | spawn with an illegal number of maxprocs using a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) (death test) |
 */

#include <limits>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/single_spawner.hpp>

using namespace std::string_literals;


TEST(SingleSpawnerTest, ConstructValid) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check of values were set correctly
    EXPECT_EQ(ss.command(), "a.out"s);
    EXPECT_EQ(ss.maxprocs(), 1);
}

TEST(SingleSpawnerDeathTest, ConstructInvalidCommand) {
    // creating single_spawner with empty command string is invalid
    ASSERT_DEATH( mpicxx::single_spawner ss(""s, 1) , "");
}

TEST(SingleSpawnerDeathTest, ConstructInvalidMaxprocs) {
    // creating single_spawner with an illegal number of maxprocs
    ASSERT_DEATH( mpicxx::single_spawner ss("a.out", -1) , "");
    ASSERT_DEATH( mpicxx::single_spawner ss("a.out", 0) , "");
    ASSERT_DEATH( mpicxx::single_spawner ss("a.out", std::numeric_limits<int>::max()) , "");
}

TEST(SingleSpawnerTest, ConstructFromPairValid) {
    // create new single_spawner object
    mpicxx::single_spawner ss(std::make_pair("a.out", 1));

    // check of values were set correctly
    EXPECT_EQ(ss.command(), "a.out"s);
    EXPECT_EQ(ss.maxprocs(), 1);
}

TEST(SingleSpawnerDeathTest, ConstructFromPairInvalidCommand) {
    // creating single_spawner with empty command string is invalid
    ASSERT_DEATH( mpicxx::single_spawner ss(std::make_pair(""s, 1)) , "");
}

TEST(SingleSpawnerDeathTest, ConstructFromPairInvalidMaxprocs) {
    // creating single_spawner with an illegal number of maxprocs
    ASSERT_DEATH( mpicxx::single_spawner ss(std::make_pair("a.out", -1)) , "");
    ASSERT_DEATH( mpicxx::single_spawner ss(std::make_pair("a.out", 0)) , "");
    ASSERT_DEATH( mpicxx::single_spawner ss({"a.out", std::numeric_limits<int>::max()}) , "");
}