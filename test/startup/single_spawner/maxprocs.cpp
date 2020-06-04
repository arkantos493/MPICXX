/**
 * @file test/startup/single_spawner/maxprocs.cpp
 * @author Marcel Breyer
 * @date 2020-06-04
 *
 * @brief Test cases for the @ref mpicxx::single_spawner::set_maxprocs(int) and @ref mpicxx::single_spawner::maxprocs() const member
 *        functions provided by the @ref mpicxx::single_spawner class.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name     | test case description                             |
 * |:-------------------|:--------------------------------------------------|
 * | SetMaxprocs        | set a new number of maxprocs                      |
 * | SetInvalidMaxprocs | set a new illegal number of maxprocs (death test) |
 * | GetMaxprocs        | get the current number of maxprocs                |
 */

#include <limits>
#include <string>

#include <gtest/gtest.h>

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

TEST(SingleSpawnerTest, GetMaxprocs) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check getter
    EXPECT_EQ(ss.maxprocs(), 1);
}