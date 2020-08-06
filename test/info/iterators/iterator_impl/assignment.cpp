/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the assignment operator of the @ref mpicxx::info::iterator and @ref mpicxx::info::const_iterator class.
 * @details Testsuite: *InfoIteratorImplTest*
 * | test case name    | test case description                                  |
 * |:------------------|:-------------------------------------------------------|
 * | AssignmentValid   | assign a valid iterator to another one                 |
 * | AssignmentInvalid | assign an invalid iterator to another one (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(InfoIteratorImplTest, AssignmentValid) {
    // create info objects and add [key, value]-pairs
    mpicxx::info info_1;
    mpicxx::info info_2;
    MPI_Info_set(info_2.get(), "key1", "value1");
    MPI_Info_set(info_2.get(), "key2", "value2");

    // assignment between non-const iterators
    mpicxx::info::iterator it = info_1.begin();
    it = info_2.begin();
    EXPECT_TRUE(it == info_2.begin());

    // assignment between const iterators
    mpicxx::info::const_iterator const_it = info_1.cbegin();
    const_it = info_2.cbegin();
    EXPECT_TRUE(const_it == info_2.cbegin());

    // assignment from non-const to const iterator
    const_it = it + 1;
    EXPECT_TRUE(const_it == info_2.cbegin() + 1);

    // assignment to singular iterator is allowed
    mpicxx::info::iterator sit;
    sit = info_2.begin();
    EXPECT_TRUE(sit == info_2.begin());
}

TEST(InfoIteratorImplDeathTest, AssignmentInvalid) {
    // create info object
    mpicxx::info info;
    mpicxx::info::iterator it = info.begin();

    // create singular iterator
    mpicxx::info::iterator sit;

    // create iterator referring to info object referring to MPI_INFO_NULL
    mpicxx::info info_null;
    mpicxx::info::iterator info_null_it = info_null.begin();
    info_null = mpicxx::info(MPI_INFO_NULL, false);

    // assignment from singular iterator is not permitted
    EXPECT_DEATH( it = sit , "");

    it = info.begin();
    // assignment from iterator referring to an info object referring to MPI_INFO_NULL is not permitted
    EXPECT_DEATH( it = info_null_it , "");
}