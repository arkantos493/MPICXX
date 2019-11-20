/**
 * @file assert_test.cpp
 * @author Marcel Breyer
 * @date 2019-11-20
 *
 * @brief Test cases for the @ref MPICXX_ASSERT() macro.
 *
 * Due to restrictions in the google test framework this is currently **not** supported.
 */

#include <gtest/gtest.h>

#include <mpicxx/utility/assert.hpp>

namespace {
    int dummy(const int i) {
        MPICXX_ASSERT(i >= 0, "Parameter must not be negative!: n = %i", i);
        return i;
    }
}

TEST(UtilityTests, CustomAssert) {

    // assertion holds: 1 >= 0
    EXPECT_EQ(dummy(1), 1);

    // assertion holds: 0 >= 0
    EXPECT_EQ(dummy(0), 0);

    // assertion is violated: -1 < 0
//    ASSERT_DEATH(dummy(-1), "Assertion.*");

}
