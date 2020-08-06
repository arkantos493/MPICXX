/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class constructors.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name                   | test case description                                                                                                                                                        |
 * |:---------------------------------|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * | ConstructValid                   | construct a new @ref mpicxx::single_spawner object                                                                                                                           |
 * | ConstructInvalidCommand          | try constructing a new @ref mpicxx::single_spawner object with an illegal executable name (death test)                                                                       |
 * | ConstructInvalidMaxprocs         | try constructing a new @ref mpicxx::single_spawner object with an illegal maxprocs value (death test)                                                                        |
 * | ConstructFromPairValid           | construct a new @ref mpicxx::single_spawner object using a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair)                                                     |
 * | ConstructFromPairInvalidCommand  | try constructing a new @ref mpicxx::single_spawner object with an illegal executable name using a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) (death test) |
 * | ConstructFromPairInvalidMaxprocs | try constructing a new @ref mpicxx::single_spawner object with an illegal maxprocs value using a [`std::pair`](https://en.cppreference.com/w/cpp/utility/pair) (death test)  |
 */

#include <mpicxx/startup/single_spawner.hpp>

#include <gtest/gtest.h>

#include <limits>
#include <string>
#include <utility>

using namespace std::string_literals;

TEST(SingleSpawnerTest, ConstructValid) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check if values were set correctly
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

    // check if values were set correctly
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