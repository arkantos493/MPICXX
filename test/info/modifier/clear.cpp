/**
 * @file test/info/modifier/clear.cpp
 * @author Marcel Breyer
 * @date 2020-02-14
 *
 * @brief Test cases for the @ref mpicxx::info::clear() member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ModifierTest*
 * | test case name | test case description                            |
 * |:---------------|:-------------------------------------------------|
 * | Clear          | remove all [key, value]-pairs from info object   |
 * | MovedFromClear | info object in the moved-from state (death test) |
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

TEST(ModifierDeathTest, MovedFromClear) {
    // create info object and set it to the moved-from state
    mpicxx::info info;
    mpicxx::info dummy(std::move(info));

    // calling clear() on an info object in the moved-from state is illegal
    ASSERT_DEATH( info.clear() , "");
}