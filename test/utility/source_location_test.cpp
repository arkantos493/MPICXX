/**
 * @file source_location_test.cpp
 * @author Marcel Breyer
 * @date 2019-11-20
 *
 * @brief Test cases for the (maybe custom) @ref mpicxx::source_location implementation.
 */

#include <gtest/gtest.h>

#include <mpicxx/utility/source_location.hpp>

TEST(UtilityTests, CurrentSourceLocation) {
    mpicxx::utility::source_location loc = mpicxx::utility::source_location::current();

    // test file name
    EXPECT_STREQ(loc.file_name(), __FILE__);

    // test function name
    EXPECT_STREQ(loc.function_name(), "TestBody");

    // test line number
    EXPECT_EQ(loc.line(), 14);

    // test column number
    EXPECT_EQ(loc.column(), 0);
}
