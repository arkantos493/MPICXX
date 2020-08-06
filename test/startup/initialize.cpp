/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the initialization functions.
 * @details Testsuite: *StartupTest*
 * | test case name | test case description                                                                                      |
 * |:---------------|:-----------------------------------------------------------------------------------------------------------|
 * | IsInitialized  | check that [*MPI_Init()*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm) has been called |
 * | IsActive       | check that the MPI environment is currently active                                                         |
 * | IsMainThread   | check whether this thread is the main thread                                                               |
 */

#include <mpicxx/startup/init.hpp>

#include <gtest/gtest.h>

TEST(StartupTest, IsInitialized) {
    // MPI should be initialized
    EXPECT_TRUE(mpicxx::initialized());
}

TEST(StartupTest, IsActive) {
    // the MPI environment should be running
    EXPECT_TRUE(mpicxx::active());
}

TEST(StartupTest, IsMainThread) {
    // check whether this thread is the main thread -> true since only one thread is spawned
    EXPECT_TRUE(mpicxx::is_main_thread());
}