/**
 * @file test/startup/single_spawner/maxprocs.cpp
 * @author Marcel Breyer
 * @date 2020-04-14
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class maxprocs member functions.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name     | test case description                                      |
 * |:-------------------|:-----------------------------------------------------------|
 * | SetMaxprocs        | set a new number of maxprocs                               |
 * | SetInvalidMaxprocs | set a new illegal number of maxprocs (death test)          |
 * | ChainSetMaxprocs   | chain calls to @ref mpicxx::single_spawner::set_maxprocs() |
 * | GetMaxprocs        | get the current number of maxprocs                         |
 * | GetUniverseSize    | get the available universe size                            |
 */

#include <limits>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/single_spawner.hpp>

using namespace std::string_literals;


TEST(SingleSpawnerTest, SetMaxprocs) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    ASSERT_EQ(ss.maxprocs(), 1);

    // set a new number of maxprocs
    ss.set_maxprocs(2);

    // check whether the number of maxprocs has been updated
    EXPECT_EQ(ss.maxprocs(), 2);
}

TEST(SingleSpawnerDeathTest, SetInvalidMaxprocs) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    ASSERT_EQ(ss.maxprocs(), 1);

    // set a new illegal number of maxprocs
    ASSERT_DEATH( ss.set_maxprocs(0) , "");
    ASSERT_DEATH( ss.set_maxprocs(-1) , "");
    ASSERT_DEATH( ss.set_maxprocs(std::numeric_limits<int>::max()) , "");
}

TEST(SingleSpawnerTest, ChainSetMaxprocs) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    ASSERT_EQ(ss.maxprocs(), 1);

    // chain multiple calls to set_maxprocs
    ss.set_maxprocs(2).set_maxprocs(1).set_maxprocs(2);

    // check whether the number of maxprocs has been updated to the last maxprocs number
    EXPECT_EQ(ss.maxprocs(), 2);
}

TEST(SingleSpawnerTest, GetMaxprocs) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check getter
    EXPECT_EQ(ss.maxprocs(), 1);
}

TEST(SingleSpawnerTest, GetUniverseSize) {
    // check universe size
    EXPECT_NE(mpicxx::single_spawner::universe_size(), 0);
}