/**
 * @file info/non-member_functions/inequality.cpp
 * @author Marcel Breyer
 * @date 2020-01-30
 *
 * @brief Test cases for the @ref mpicxx::info::operator!=(const info&, const info&) function provided by the @ref mpicxx::info class.
 * @details Testsuite: *NonMemberFunctionTest*
 * | test case name        | test case description                               |
 * |:----------------------|:----------------------------------------------------|
 * | Inequality            | check various `!=` cases                            |
 * | InequalityIdempotence | `info1 != info1; // false`                          |
 * | InequalitySymmetry    | `info1 != info2` <-> `info2 != info1`               |
 * | InequalityNonFreeable | freeable state should't have any impact on equality |
 * | MovedFromInequality   | info objects in the moved-from state (death test)   |
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(NonMemberFunctionTest, Inequality) {
    // create two empty info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // info objects shouldn't compare inequal
    EXPECT_FALSE(info_1 != info_2);

    // add a [key, value]-pair to one info object
    MPI_Info_set(info_1.get(), "key", "value");

    // info objects should compare inequal
    EXPECT_TRUE(info_1 != info_2);

    // add a [key, value]-pair with the same key, but a different value, to the other info object
    MPI_Info_set(info_2.get(), "key", "other_value");

    // info objects should still compare inequal
    EXPECT_TRUE(info_1 != info_2);

    // change value in info_2 to match the one of info_1
    MPI_Info_set(info_2.get(), "key", "value");

    // info objects shouldn't compare inequal again
    EXPECT_FALSE(info_1 != info_2);

    // remove all [key, value]-pairs
    MPI_Info_delete(info_1.get(), "key");
    MPI_Info_delete(info_2.get(), "key");

    // info objects shouldn't compare inequal
    EXPECT_FALSE(info_1 != info_2);
}

TEST(NonMemberFunctionTest, InequalityIdempotence) {
    // create empty info object
    mpicxx::info info_1;

    // info object shouldn't compare inequal with itself
    EXPECT_FALSE(info_1 != info_1);

    // add a [key, value]-pair to the info object
    MPI_Info_set(info_1.get(), "key", "value");

    // info object shouldn't compare inequal with itself
    EXPECT_FALSE(info_1 != info_1);
}

TEST(NonMemberFunctionTest, InequalitySymmetry) {
    // create two empty info objects
    mpicxx::info info_1;
    mpicxx::info info_2;

    // info objects should't compare inequal
    EXPECT_FALSE(info_1 != info_2);
    EXPECT_FALSE(info_2 != info_1);

    // add a [key, value]-pair to one info object
    MPI_Info_set(info_1.get(), "key", "value");

    // info objects should compare inequal
    EXPECT_TRUE(info_1 != info_2);
    EXPECT_TRUE(info_2 != info_1);
}

TEST(NonMemberFunctionTest, InequalityNonFreeable) {
    // create info object (freeable and non-freeable)
    mpicxx::info freeable;
    MPI_Info info_2;
    MPI_Info_create(&info_2);
    mpicxx::info non_freeable(info_2, false);
    ASSERT_FALSE(freeable.freeable() == non_freeable.freeable());

    // the freeable state should't have any effect
    EXPECT_FALSE(freeable != non_freeable);

    MPI_Info_free(&info_2);
}

TEST(NonMemberFunctionDeathTest, MovedFromInequality) {
    // create two info objects and set them to the moved-from state
    mpicxx::info moved_from_1;
    mpicxx::info valid_1(std::move(moved_from_1));
    mpicxx::info moved_from_2;
    mpicxx::info valid_2(std::move(moved_from_2));

    ASSERT_FALSE(valid_1 != valid_2);
    // comparing two moved-from info objects is illegal
    [[maybe_unused]] bool comp;
    ASSERT_DEATH(comp = moved_from_1 != valid_1, "");
    ASSERT_DEATH(comp = moved_from_2 != valid_2, "");
    ASSERT_DEATH(comp = moved_from_1 != moved_from_2, "");
}