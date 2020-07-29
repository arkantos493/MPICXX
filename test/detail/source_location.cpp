/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::detail::source_location implementation.
 * @details Testsuite: *DetailTest*
 * | test case name               | test case description                                          |
 * |:-----------------------------|:---------------------------------------------------------------|
 * | SourceLocation               | test the source location information                           |
 * | SourceLocationPrettyFuncName | test the source location information with pretty function name |
 * | SourceLocationStackTrace     | test the source location information with pretty function name |
 */

#include <mpicxx/detail/source_location.hpp>

#include <gtest/gtest.h>

#include <sstream>

TEST(DetailTest, CurrentSourceLocation) {
    mpicxx::detail::source_location loc = mpicxx::detail::source_location::current();

    // test file name
    EXPECT_STREQ(loc.file_name(), __FILE__);

    // test function name
    EXPECT_STREQ(loc.function_name(), "TestBody");

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
    mpicxx::detail::source_location loc = mpicxx::detail::source_location::current(MPICXX_PRETTY_FUNC_NAME__);

    // test file name
    EXPECT_STREQ(loc.file_name(), __FILE__);

    // test function name
    EXPECT_STREQ(loc.function_name(), "virtual void DetailTest_CurrentSourceLocationPrettyFuncName_Test::TestBody()");

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

    std::string trace = loc.stack_trace();

#if defined(MPICXX_ENABLE_STACK_TRACE) && defined(__GNUG__)
    // stack trace should be present
    EXPECT_FALSE(trace.empty());
#elif defined(MPICXX_ENABLED_STACK_TRACE) && !defined(__GNUG__)
    // no stack trace supported
    using namespace std::string_literals;
    EXPECT_EQ(trace, "No stack trace supported!"s)
#else
    // no stack trace requested
    EXPECT_TRUE(trace.empty());
#endif

}