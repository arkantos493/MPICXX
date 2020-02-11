/**
 * @file info/iterators/iterator_impl/distance.cpp
 * @author Marcel Breyer
 * @date 2020-02-11
 *
 * @brief Test cases for the distance calculation of the @ref mpicxx::info::iterator and @ref mpicxx::info::const_iterator class.
 * @details Testsuite: *IteratorImplTest*
 * | test case name  | test case description                                             |
 * |:----------------|:------------------------------------------------------------------|
 * | DistanceValid   | calculate the distance between two valid iterators                |
 * | DistanceInvalid | calculate the distance between two invalid iterators (death test) |
 */


#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(IteratorImplTest, DistanceValid) {
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

TEST(IteratorImplDeathTest, DistanceInvalid) {
    // create info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // create singular iterators
    mpicxx::info::iterator sit_1;
    mpicxx::info::iterator sit_2;

    // create iterators referring to moved-from info objects
    mpicxx::info moved_from_1;
    mpicxx::info moved_from_2;
    mpicxx::info::iterator moved_from_it_1 = moved_from_1.begin();
    mpicxx::info::iterator moved_from_it_2 = moved_from_2.begin();
    mpicxx::info dummy_1(std::move(moved_from_1));
    mpicxx::info dummy_2(std::move(moved_from_2));

    [[maybe_unused]] int dist;
    // distance calculation between singular iterators is not permitted
    EXPECT_DEATH( dist = sit_1 - sit_2 , "");
    EXPECT_DEATH( dist = sit_1 - info_1.begin() , "");
    EXPECT_DEATH( dist = info_1.begin() - sit_1 , "");

    // distance calculation between iterators referring to info objects in the moved-from state is not permitted
    EXPECT_DEATH( dist = moved_from_it_1 - moved_from_it_2 , "");
    EXPECT_DEATH( dist = moved_from_it_1 - info_1.begin() , "");
    EXPECT_DEATH( dist = info_1.begin() - moved_from_it_1 , "");

    // distance calculation between iterators from different info objects is not permitted
    EXPECT_DEATH( dist = info_1.begin() - info_2.end() , "");
}