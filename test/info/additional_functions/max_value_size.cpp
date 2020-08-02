/**
 * @file 
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::max_value_size() static member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name | test case description |
 * |:---------------|:----------------------|
 * | MaxValueSize   | check received value  |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(NonMemberFunctionTest, MaxValueSize) {
    // static access
    EXPECT_EQ(mpicxx::info::max_value_size(), MPI_MAX_INFO_VAL);

    // access via info object
    mpicxx::info info;
    EXPECT_EQ(info.max_value_size(), MPI_MAX_INFO_VAL);

    // access via info object which has been moved
    mpicxx::info dummy(std::move(info));
    EXPECT_EQ(info.max_value_size(), MPI_MAX_INFO_VAL);
}