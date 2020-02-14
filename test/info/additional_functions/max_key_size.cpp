/**
 * @file test/info/additional_functions/max_key_size.cpp
 * @author Marcel Breyer
 * @date 2020-02-14
 *
 * @brief Test cases for the @ref mpicxx::info::max_key_size() static member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name | test case description                            |
 * |:---------------|:-------------------------------------------------|
 * | MaxKeySize     | check received value                             |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, MaxKeySize) {
    // static access
    EXPECT_EQ(mpicxx::info::max_key_size(), MPI_MAX_INFO_KEY);

    // access via info object
    mpicxx::info info;
    EXPECT_EQ(info.max_key_size(), MPI_MAX_INFO_KEY);

    // access via info object in the moved-from state
    mpicxx::info dummy(std::move(info));
    EXPECT_EQ(info.max_key_size(), MPI_MAX_INFO_KEY);
}