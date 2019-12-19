/**
 * @file inequality.cpp
 * @author Marcel Breyer
 * @date 2019-12-16
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the `operator!=` non-member function of the mpicxx::info class.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, InequalityEqual) {
    // create two empty info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // info objects should not compare inequal
    EXPECT_FALSE(info_1 != info_2);

    // add the same [key, value]-pair to each info object
    MPI_Info_set(info_1.get(), "key", "value");
    MPI_Info_set(info_2.get(), "key", "value");

    // info objects should still not compare inequal
    EXPECT_FALSE(info_1 != info_2);
}

TEST(NonMemberFunctionTest, InequalityInequal) {
    // create two empty info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // info objects should not compare inequal
    EXPECT_FALSE(info_1 != info_2);

    // add the [key, value]-pair to one info object
    MPI_Info_set(info_1.get(), "key", "value");

    // info objects should compare inequal
    EXPECT_TRUE(info_1 != info_2);
}

TEST(NonMemberFunctionTest, MovedFromInequality) {
    // create two empty info objects and set them to the "moved-from" state
    mpicxx::info info_1;
    mpicxx::info dummy_1(std::move(info_1));
    mpicxx::info info_2;
    mpicxx::info dummy_2(std::move(info_2));

    // comparing "moved-from" objects is no supported
//    [[maybe_unused]] const bool is_inequal_1 = info_1  != info_2;        // -> should assert
//    [[maybe_unused]] const bool is_inequal_2 = dummy_1 != info_2;        // -> should assert
//    [[maybe_unused]] const bool is_inequal_3 = info_1  != dummy_2;       // -> should assert
}