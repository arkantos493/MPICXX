/**
 * @file test/info/null.cpp
 * @author Marcel Breyer
 * @date 2020-03-20
 *
 * @brief Test cases for the @ref mpicxx::info::null static const info object provided by the @ref mpicxx::info class.
 * @details Testsuite: *InfoNullTest*
 * | test case name | test case description            |
 * |:---------------|:---------------------------------|
 * | InfoNull       | check if same as *MPI_INFO_NULL* |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoEnvTest, InfoNull) {
    // check whether mpicxx::info::null is the same as MPI_INFO_NULL
    EXPECT_EQ(mpicxx::info::null.get(), MPI_INFO_NULL);

    // it should not be freeable
    EXPECT_FALSE(mpicxx::info::null.freeable());
}