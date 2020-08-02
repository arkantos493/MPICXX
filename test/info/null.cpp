/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::null static const info object provided by the @ref mpicxx::info class.
 * @details Testsuite: *InfoNullTest*
 * | test case name | test case description                                                                            |
 * |:---------------|:-------------------------------------------------------------------------------------------------|
 * | InfoNull       | check against [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(InfoNullTest, InfoNull) {
    // check for correctness
    EXPECT_EQ(mpicxx::info::null.get(), MPI_INFO_NULL);
    EXPECT_FALSE(mpicxx::info::null.freeable());
}