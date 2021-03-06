/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::empty() const member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *CapacityTest*
 * | test case name | test case description                                                                                                    |
 * |:---------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | Empty          | empty info object                                                                                                        |
 * | NonEmpty       | non empty info object                                                                                                    |
 * | NullEmpty      | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(CapacityTest, Empty) {
    // create empty info object
    mpicxx::info info;

    // info object is empty
    EXPECT_TRUE(info.empty());
}

TEST(CapacityTest, NonEmpty) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");

    // info object is not empty
    EXPECT_FALSE(info.empty());

    // add second element
    MPI_Info_set(info.get(), "key2", "value2");

    // info object is still not empty
    EXPECT_FALSE(info.empty());

    // delete both elements
    MPI_Info_delete(info.get(), "key1");
    MPI_Info_delete(info.get(), "key2");

    // info object is now empty
    EXPECT_TRUE(info.empty());
}

TEST(CapacityDeathTest, NullEmpty) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling empty() on an info object referring to MPI_INFO_NULL is illegal
    [[maybe_unused]] bool empty;
    ASSERT_DEATH( empty = info.empty() , "");
}