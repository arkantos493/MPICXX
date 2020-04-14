/**
 * @file test/startup/single_spawner/universe_size.cpp
 * @author Marcel Breyer
 * @date 2020-04-14
 *
 * @brief Test cases for the @ref mpicxx::single_spawner class universe size member functions.
 * @details Testsuite: *SingleSpawnerTest*
 * | test case name  | test case description           |
 * |:----------------|:--------------------------------|
 * | GetUniverseSize | get the available universe size |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/startup/single_spawner.hpp>


TEST(SingleSpawnerTest, GetUniverseSize) {
    // check universe size
    EXPECT_NE(mpicxx::single_spawner::universe_size(), 0);
}