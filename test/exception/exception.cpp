/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::exception class.
 * @details Testsuite: *ExceptionTest*
 * | test case name                       | test case description                                          |
 * |:-------------------------------------|:---------------------------------------------------------------|
 * | ThrowException                       | throw a base exception with source location information        |
 * | ThrowExceptionWithPrettyFunctionName | throw a base exception with better source location information |
 */

#include <mpicxx/exception/exception.hpp>

#include <gtest/gtest.h>

namespace {

    void function_that_throws() {
        throw mpicxx::exception();
    }

    void function_that_pretty_throws() {
        MPICXX_THROW_EXCEPTION(mpicxx::exception);
    }

}

TEST(ExceptionTest, ThrowException) {
    // exception should be thrown
    try {
        function_that_throws();
        FAIL() << "expected mpicxx::exception";
    } catch (const mpicxx::exception& e) {
        EXPECT_STREQ(e.location().file_name(), __FILE__);
        EXPECT_STREQ(e.location().function_name(), "function_that_throws");
        ASSERT_TRUE(e.location().rank().has_value());
        EXPECT_EQ(e.location().rank().value(), 0);
    } catch (...) {
        FAIL() << "expected mpicxx::exception";
    }
}

TEST(ExceptionTest, ThrowExceptionWithPrettyFunctionName) {
    // exception should be thrown
    try {
        function_that_pretty_throws();
        FAIL() << "expected mpicxx::exception";
    } catch (const mpicxx::exception& e) {
        EXPECT_STREQ(e.location().file_name(), __FILE__);
        EXPECT_STREQ(e.location().function_name(), "void {anonymous}::function_that_pretty_throws()");
        ASSERT_TRUE(e.location().rank().has_value());
        EXPECT_EQ(e.location().rank().value(), 0);
    } catch (...) {
        FAIL() << "expected mpicxx::exception";
    }
}