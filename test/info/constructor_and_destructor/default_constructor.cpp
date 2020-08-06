/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::info() member function provided by the @ref mpicxx::info class.
 * @details Testsuite: *ConstructionTest*
 * | test case name      | test case description         |
 * |:--------------------|:------------------------------|
 * | DefaultConstruction | default construct info object |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(ConstructionTest, DefaultConstruction) {
    // default construct an info object
    mpicxx::info info;

    // a default constructed info object should not refer to MPI_INFO_NULL
    ASSERT_NE(info.get(), MPI_INFO_NULL);

    // a default constructed info object should be emtpy
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // a default constructed info object is always freeable
    EXPECT_TRUE(info.freeable());
}