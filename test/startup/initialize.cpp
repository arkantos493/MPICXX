/**
 * @file test/startup/initialize.cpp
 * @author Marcel Breyer
 * @date 2020-02-26
 *
 * @brief Test cases for the @ref mpicxx::initialized() function.
 * @details Testsuite: *StartupTest*
 * | test case name | test case description                               |
 * |:---------------|:----------------------------------------------------|
 * | IsInitialized  | check that *MPI_Init()* has been called             |
 * | IsRunning      | check that the MPI environment is currently running |
 * | IsMainThread   | check whether this thread is the main thread        |
 */

#include <gtest/gtest.h>

#include <mpicxx/startup/init.hpp>


TEST(StartupTest, IsInitialized) {
    // MPI should be initialized
    EXPECT_TRUE(mpicxx::initialized());
}

TEST(StartupTest, IsRunning) {
    // the MPI environment should be running
    EXPECT_TRUE(mpicxx::running());
}

TEST(StartupTest, IsMainThread) {
    // check whether this thread is the main thread -> true since only one thread is spawned
    EXPECT_TRUE(mpicxx::is_main_thread());
}