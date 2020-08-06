/**
 * @file 
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::max_size() static member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *CapacityTest*
 * | test case name | test case description                                                                                                               |
 * |:---------------|:------------------------------------------------------------------------------------------------------------------------------------|
 * | MaxSize        | get max size through an info object                                                                                                 |
 * | NullMaxSize    | get max size through an info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) |
 * | StaticMaxSize  | get max_size through a static function call                                                                                         |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <limits>

TEST(CapacityTest, MaxSize) {
    // create info object
    mpicxx::info info;

    // get the maximum possible number of [key, value]-pairs
    EXPECT_EQ(info.max_size(), std::numeric_limits<typename mpicxx::info::difference_type>::max());
}

TEST(CapacityTest, NullMaxSize) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling max_size() on an info object referring to MPI_INFO_NULL is valid
    EXPECT_EQ(info.max_size(), std::numeric_limits<typename mpicxx::info::difference_type>::max());
}

TEST(CapacityTest, StaticMaxSize) {
    // get the maximum possible number of [key, value]-pairs
    EXPECT_EQ(mpicxx::info::max_size(), std::numeric_limits<typename mpicxx::info::difference_type>::max());
}