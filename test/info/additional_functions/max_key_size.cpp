/**
 * @file max_key_size.cpp
 * @author Marcel Breyer
 * @date 2020-01-25
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `max_key_size` member function provided for a mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, MaxKeySize) {
    ASSERT_EQ(mpicxx::info::max_key_size(), MPI_MAX_INFO_KEY);
}