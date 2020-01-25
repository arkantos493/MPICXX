/**
 * @file max_value_size.cpp
 * @author Marcel Breyer
 * @date 2020-01-25
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `max_value_size` member function provided for a mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, MaxValueSize) {
    ASSERT_EQ(mpicxx::info::max_value_size(), MPI_MAX_INFO_VAL);
}