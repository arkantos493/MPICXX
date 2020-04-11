/**
 * @file test/info/modifier/clear.cpp
 * @author Marcel Breyer
 * @date 2020-04-11
 *
 * @brief Test cases for the @ref mpicxx::info::clear() member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name | test case description                               |
 * |:---------------|:----------------------------------------------------|
 * | Clear          | remove all [key, value]-pairs from info object      |
 * | NullClear      | info object referring to MPI_INFO_NULL (death test) |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


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