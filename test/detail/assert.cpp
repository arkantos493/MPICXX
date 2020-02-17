/**
 * @file detail/assert.cpp
 * @author Marcel Breyer
 * @date 2020-02-08
 *
 * @brief Test cases for the MPICXX_ASSERTION macros.
 * @details Testsuite: *DetailTest*
 * | test case name               | test case description                   |
 * |:-----------------------------|:----------------------------------------|
 * | AssertPreconditionHolds      | assert precondition checks              |
 * | AssertPreconditionDoesntHold | assert precondition checks (death test) |
 * | AssertSanityHolds            | assert sanity checks                    |
 * | AssertSanityDoesntHold       | assert sanity checks (death test)       |
 */

#include <gtest/gtest.h>

#include <mpicxx/detail/assert.hpp>

namespace {
    int precondition_check(const int i) {
        MPICXX_ASSERT_PRECONDITION(i >= 0, "Parameter must not be negative!: n = %i", i);
        return i;
    }

    int sanity_check(const int i) {
        MPICXX_ASSERT_SANITY(i >= 0, "Parameter must not be negative!: n = %i", i);
        return i;
    }
}

TEST(DetailTest, AssertPreconditionHolds) {
    // assertion holds: 1 >= 0
    EXPECT_EQ(precondition_check(1), 1);

    // assertion holds: 0 >= 0
    EXPECT_EQ(precondition_check(0), 0);
}

TEST(DetailDeathTest, AssertPreconditionDoesntHold) {
    // assertion violated: -2 < 0
    ASSERT_DEATH(precondition_check(-2), "");
}

TEST(DetailTest, AssertSanityHolds) {
    // assertion holds: 1 >= 0
    EXPECT_EQ(sanity_check(1), 1);

    // assertion holds: 0 >= 0
    EXPECT_EQ(sanity_check(0), 0);
}

TEST(DetailDeathTest, AssertSanityDoesntHold) {
    // assertion violated: -2 < 0
    ASSERT_DEATH(sanity_check(-2), "");
}