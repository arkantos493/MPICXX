/**
 * @file 
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::thread_support_not_satisfied exception class.
 * @details Testsuite: *ExceptionTest*
 * | test case name                                                | test case description                                   |
 * |:--------------------------------------------------------------|:--------------------------------------------------------|
 * | ThrowThreadSupportNotSatisfiedException                       | throw exception with source location information        |
 * | ThrowThreadSupportNotSatisfiedExceptionWithPrettyFunctionName | throw exception with better source location information |
 */

#include <mpicxx/exception/thread_support_exception.hpp>

#include <gtest/gtest.h>

namespace {

    void function_that_throws() {
        throw mpicxx::thread_support_not_satisfied(mpicxx::thread_support::multiple, mpicxx::thread_support::single);
    }

    void function_that_pretty_throws() {
        MPICXX_THROW_EXCEPTION(mpicxx::thread_support_not_satisfied, mpicxx::thread_support::multiple, mpicxx::thread_support::single);
    }

}

TEST(ExceptionTest, ThrowThreadSupportNotSatisfiedException) {
    // exception should be thrown
    try {
        function_that_throws();
        FAIL() << "expected mpicxx::thread_support_not_satisfied exception";
    } catch (const mpicxx::thread_support_not_satisfied& e) {
        EXPECT_STREQ(e.location().file_name(), __FILE__);
        EXPECT_STREQ(e.location().function_name(), "function_that_throws");
        ASSERT_TRUE(e.location().rank().has_value());
        EXPECT_EQ(e.location().rank().value(), 0);
        EXPECT_EQ(e.required(), mpicxx::thread_support::multiple);
        EXPECT_EQ(e.provided(), mpicxx::thread_support::single);
    } catch (...) {
        FAIL() << "expected mpicxx::thread_support_not_satisfied exception";
    }
}

TEST(ExceptionTest, ThrowThreadSupportNotSatisfiedExceptionWithPrettyFunctionName) {
    // exception should be thrown
    try {
        function_that_pretty_throws();
        FAIL() << "expected mpicxx::thread_support_not_satisfied exception";
    } catch (const mpicxx::thread_support_not_satisfied& e) {
        EXPECT_STREQ(e.location().file_name(), __FILE__);
        EXPECT_STREQ(e.location().function_name(), "void {anonymous}::function_that_pretty_throws()");
        ASSERT_TRUE(e.location().rank().has_value());
        EXPECT_EQ(e.location().rank().value(), 0);
        EXPECT_EQ(e.required(), mpicxx::thread_support::multiple);
        EXPECT_EQ(e.provided(), mpicxx::thread_support::single);
    } catch (...) {
        FAIL() << "expected mpicxx::thread_support_not_satisfied exception";
    }
}