/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::clear() member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name | test case description                                                                                                    |
 * |:---------------|:-------------------------------------------------------------------------------------------------------------------------|
 * | Clear          | remove all [key, value]-pairs from info object                                                                           |
 * | NullClear      | info object referring to [*MPI_INFO_NULL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(ModifierTest, Clear) {
    // create info object
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");
    MPI_Info_set(info.get(), "key3", "value3");

    // size should be three
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 3);

    // clear info object
    info.clear();

    // now the info object should be empty
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // clearing again should do nothing
    info.clear();
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);
}

TEST(ModifierDeathTest, NullClear) {
    // create null info object
    mpicxx::info info(MPI_INFO_NULL, false);

    // calling clear() on an info object referring to MPI_INFO_NULL is illegal
    ASSERT_DEATH( info.clear() , "");
}