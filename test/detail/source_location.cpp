/**
 * @file test/detail/source_location.cpp
 * @author Marcel Breyer
 * @date 2020-02-08
 *
 * @brief Test cases for the @ref mpicxx::detail::source_location implementation.
 * @details Testsuite: *DetailTest*
 * | test case name               | test case description                                          |
 * |:-----------------------------|:---------------------------------------------------------------|
 * | SourceLocation               | test the source location information                           |
 * | SourceLocationPrettyFuncName | test the source location information with pretty function name |
 * | SourceLocationStackTrace     | test the source location information with pretty function name |
 */

#include <sstream>

#include <gtest/gtest.h>

#include <mpicxx/detail/source_location.hpp>

TEST(DetailTest, CurrentSourceLocation) {
    mpicxx::detail::source_location loc = mpicxx::detail::source_location::current();

    // test file name
    EXPECT_STREQ(loc.file_name().c_str(), __FILE__);

    // test function name
    EXPECT_STREQ(loc.function_name().c_str(), "TestBody");

    // test line number
    EXPECT_EQ(loc.line(), 22);

    // test column number
    EXPECT_EQ(loc.column(), 0);

    // test MPI rank
    ASSERT_TRUE(loc.rank().has_value());
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    EXPECT_EQ(loc.rank().value(), rank);
}

TEST(DetailTest, CurrentSourceLocationPrettyFuncName) {
    mpicxx::detail::source_location loc = mpicxx::detail::source_location::current(PRETTY_FUNC_NAME__);

    // test file name
    EXPECT_STREQ(loc.file_name().c_str(), __FILE__);

    // test function name
    EXPECT_STREQ(loc.function_name().c_str(), "virtual void DetailTest_CurrentSourceLocationPrettyFuncName_Test::TestBody()");

    // test line number
    EXPECT_EQ(loc.line(), 44);

    // test column number
    EXPECT_EQ(loc.column(), 0);

    // test MPI rank
    ASSERT_TRUE(loc.rank().has_value());
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    EXPECT_EQ(loc.rank().value(), rank);
}

TEST(DetailTest, SourceStackTrace) {
    mpicxx::detail::source_location loc = mpicxx::detail::source_location::current();

    std::stringstream ss;
    loc.stack_trace(ss);

#ifdef __GNUG__
    // stack trace should be present
    EXPECT_FALSE(ss.str().empty());
#else
    // no stack trace  supported
    EXPECT_TRUE(ss.str().empty());
#endif

}