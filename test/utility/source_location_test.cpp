#include <gtest/gtest.h>

#include <mpicxx/utility/source_location.hpp>

TEST(UtilityTests, CurrentSourceLocation) {
    mpicxx::source_location loc = mpicxx::source_location::current();

    // test file name
    EXPECT_STREQ(loc.file_name(), __FILE__);

    // test function name
    EXPECT_STREQ(loc.function_name(), "TestBody");

    // test line number
    EXPECT_EQ(loc.line(), 6);

    // test column number
    EXPECT_EQ(loc.column(), 0);
}
