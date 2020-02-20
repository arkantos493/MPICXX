/**
 * @file test/startup/initialize.cpp
 * @author Marcel Breyer
 * @date 2020-02-20
 *
 * @brief Test cases for the @ref mpicxx::initialized() function.
 * @details Testsuite: *StartupTest*
 * | test case name | test case description                               |
 * |:---------------|:----------------------------------------------------|
 * | IsInitialized  | check that *MPI_Init()* has been called             |
 * | IsRunning      | check that the MPI environment is currently running |
 */

#include <gtest/gtest.h>

#include <mpicxx/startup/initialization.hpp>


TEST(StartupTest, IsInitialized) {
    // MPI should be initialized
    EXPECT_TRUE(mpicxx::initialized());
}

TEST(StartupTest, IsRunning) {
    // the MPI environment should be running
    EXPECT_TRUE(mpicxx::running());
}