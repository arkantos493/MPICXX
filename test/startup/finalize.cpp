/**
 * @file test/startup/finalize.cpp
 * @author Marcel Breyer
 * @date 2020-02-26
 *
 * @brief Test cases for the @ref mpicxx::finalized() function.
 * @details Testsuite: *StartupTest*
 * | test case name | test case description                              |
 * |:---------------|:---------------------------------------------------|
 * | IsFinalized    | check that no *MPI_Finalize()* has been called yet |
 * | Abort          | abort the given communicator group (death test)    |
 */

#include <gtest/gtest.h>

#include <mpicxx/startup/finalize.hpp>


TEST(StartupTest, IsFinalized) {
    // MPI shouldn't be finalized yet
    EXPECT_FALSE(mpicxx::finalized());
}

TEST(StartupDeathTest, Abort) {
    // try an abort
    EXPECT_DEATH( mpicxx::abort(), "" );
}