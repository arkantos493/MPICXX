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
 */

#include <gtest/gtest.h>

#include <mpicxx/startup/finalize.hpp>


TEST(StartupTest, IsFinalized) {
    // MPI shouldn't be finalized yet
    EXPECT_FALSE(mpicxx::finalized());
}