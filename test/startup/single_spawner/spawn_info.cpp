/**
 * @file test/startup/single_spawner/spawn_info.cpp
 * @author Marcel Breyer
 * @date 2020-06-04
 *
 * @brief Test cases for the @ref mpicxx::single_spawner::set_spawn_info(info) and @ref mpicxx::single_spawner::spawn_info() const member
 *        functions provided by the @ref mpicxx::single_spawner class.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name    | test case description                            |
 * |:------------------|:-------------------------------------------------|
 * | SetSpawnInfo      | set a new @ref mpicxx::info object as spawn info |
 * | GetSpawnInfo      | get the current spawn @ref mpicxx::info object   |
 */

#include <string>

#include <gtest/gtest.h>

#include <mpicxx/startup/single_spawner.hpp>

using namespace std::string_literals;


TEST(SingleSpawnerTest, SetSpawnInfo) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check default spawn info object to refer to MPI_INFO_NULL
    ASSERT_EQ(ss.spawn_info().get(), MPI_INFO_NULL);

    // set a new spawn info object
    mpicxx::info info(MPI_INFO_ENV, false);
    ss.set_spawn_info(info);

    // check whether the spawn info object has been updated
    EXPECT_EQ(ss.spawn_info(), mpicxx::info::env);
}

TEST(SingleSpawnerTest, GetSpawnInfo) {
    // create new single_spawner object
    mpicxx::single_spawner ss("a.out", 1);

    // check getter
    EXPECT_EQ(ss.spawn_info(), mpicxx::info::null);
}