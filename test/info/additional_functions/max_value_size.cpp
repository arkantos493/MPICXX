/**
 * @file info/additional_functions/max_value_size.cpp
 * @author Marcel Breyer
 * @date 2020-01-27
 *
 * @brief Test cases for the @ref mpicxx::info::max_value_size() static member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name | test case description                            |
 * |:---------------|:-------------------------------------------------|
 * | MaxValueSize   | check received value                             |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, MaxValueSize) {
    // static access
    EXPECT_EQ(mpicxx::info::max_value_size(), MPI_MAX_INFO_VAL);

    // access via info object
    mpicxx::info info;
    EXPECT_EQ(info.max_value_size(), MPI_MAX_INFO_VAL);

    // access via info object in the moved-from state
    mpicxx::info dummy(std::move(info));
    EXPECT_EQ(info.max_value_size(), MPI_MAX_INFO_VAL);
}