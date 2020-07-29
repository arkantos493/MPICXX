/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the finalization functions.
 * @details Testsuite: *StartupTest*
 * | test case name             | test case description                              |
 * |:---------------------------|:---------------------------------------------------|
 * | IsFinalized                | check that no [*MPI_Finalize()*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node225.htm) has been called yet |
 * | Abort                      | abort the given communicator group (death test)    |
 * | AtfinalizeNullptr          | nullptr as atfinalize callback (death test)        |
 * | AtfinalizeTooManyCallbacks | nullptr as atfinalize callback (death test)        |
 */

#include <gtest/gtest.h>

#include <mpicxx/startup/finalize.hpp>

TEST(StartupTest, IsFinalized) {
    // MPI shouldn't be finalized yet
    EXPECT_FALSE(mpicxx::finalized());
}

TEST(StartupDeathTest, Abort) {
    // try an abort
    EXPECT_DEATH( mpicxx::abort() , "" );
}

TEST(StartupDeathTest, AtfinalizeNullptr) {
    // try to register a nullptr as callback function
    EXPECT_DEATH( mpicxx::atfinalize(nullptr) , "");
}

TEST(StartupDeathTest, AtfinalizeTooManyCallbacks) {
    // register MPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS number of callback functions
    const auto callback = [](){};
    for (int i = 0; i < MPICXX_MAX_NUMBER_OF_ATFINALIZE_CALLBACKS; ++i) {
        mpicxx::atfinalize(callback);
    }

    // try register one more callback function
    EXPECT_DEATH( mpicxx::atfinalize(callback) , "");
}