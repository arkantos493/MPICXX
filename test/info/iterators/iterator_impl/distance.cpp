/**
 * @file 
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the distance calculation of the @ref mpicxx::info::iterator and @ref mpicxx::info::const_iterator class.
 * @details Testsuite: *InfoIteratorImplTest*
 * | test case name  | test case description                                             |
 * |:----------------|:------------------------------------------------------------------|
 * | DistanceValid   | calculate the distance between two valid iterators                |
 * | DistanceInvalid | calculate the distance between two invalid iterators (death test) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(InfoIteratorImplTest, DistanceValid) {
    // create info object and add [key, value]-pairs
    mpicxx::info info;
    MPI_Info_set(info.get(), "key1", "value1");
    MPI_Info_set(info.get(), "key2", "value2");

    // check distance calculation
    EXPECT_EQ(info.begin() - info.end(), -2);
    EXPECT_EQ(info.end() - info.begin(), 2);

    EXPECT_EQ(info.begin() - info.begin(), 0);
    EXPECT_EQ(info.end() - info.end(), 0);
}

TEST(InfoIteratorImplDeathTest, DistanceInvalid) {
    // create info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // create singular iterators
    mpicxx::info::iterator sit_1;
    mpicxx::info::iterator sit_2;

    // create iterators referring to info objects referring to MPI_INFO_NULL
    mpicxx::info info_null_1;
    mpicxx::info info_null_2;
    mpicxx::info::iterator info_null_it_1 = info_null_1.begin();
    mpicxx::info::iterator info_null_it_2 = info_null_2.begin();
    info_null_1 = mpicxx::info(MPI_INFO_NULL, false);
    info_null_2 = mpicxx::info(MPI_INFO_NULL, false);

    [[maybe_unused]] int dist;
    // distance calculation between singular iterators is not permitted
    EXPECT_DEATH( dist = sit_1 - sit_2 , "");
    EXPECT_DEATH( dist = sit_1 - info_1.begin() , "");
    EXPECT_DEATH( dist = info_1.begin() - sit_1 , "");

    // distance calculation between iterators referring to info objects referring to MPI_INFO_NULL is not permitted
    EXPECT_DEATH( dist = info_null_it_1 - info_null_it_2 , "");
    EXPECT_DEATH( dist = info_null_it_1 - info_1.begin() , "");
    EXPECT_DEATH( dist = info_1.begin() - info_null_it_1 , "");

    // distance calculation between iterators from different info objects is not permitted
    EXPECT_DEATH( dist = info_1.begin() - info_2.end() , "");
}