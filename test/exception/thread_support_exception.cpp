/**
 * @file test/exception/thread_support_exception.cpp
 * @author Marcel Breyer
 * @date 2020-03-03
 *
 * @brief Test cases for the @ref mpicxx::thread_support_not_satisfied exception class.
 * @details Testsuite: *ExceptionTest*
 * | test case name                                                | test case description                                   |
 * |:--------------------------------------------------------------|:--------------------------------------------------------|
 * | ThrowThreadSupportNotSatisfiedException                       | throw exception with source location information        |
 * | ThrowThreadSupportNotSatisfiedExceptionWithPrettyFunctionName | throw exception with better source location information |
 */

#include <gtest/gtest.h>

#include <mpicxx/exception/thread_support_exception.hpp>


namespace {

    void function_that_throws() {
        throw mpicxx::thread_support_not_satisfied(mpicxx::thread_support::MULTIPLE, mpicxx::thread_support::SINGLE);
    }

    void function_that_pretty_throws() {
        throw_with_pretty_location(mpicxx::thread_support_not_satisfied, mpicxx::thread_support::MULTIPLE, mpicxx::thread_support::SINGLE);
    }

}

TEST(ExceptionTest, ThrowThreadSupportNotSatisfiedException) {
    // exception should be thrown
    try {
        function_that_throws();
        FAIL() << "expected mpicxx::thread_support_not_satisfied exception";
    } catch (const mpicxx::thread_support_not_satisfied& e) {
        EXPECT_STREQ(e.location().file_name().c_str(), __FILE__);
        EXPECT_STREQ(e.location().function_name().c_str(), "function_that_throws");
        ASSERT_TRUE(e.location().rank().has_value());
        EXPECT_EQ(e.location().rank().value(), 0);
        EXPECT_EQ(e.required(), mpicxx::thread_support::MULTIPLE);
        EXPECT_EQ(e.provided(), mpicxx::thread_support::SINGLE);
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
        EXPECT_STREQ(e.location().file_name().c_str(), __FILE__);
        EXPECT_STREQ(e.location().function_name().c_str(), "void {anonymous}::function_that_pretty_throws()");
        ASSERT_TRUE(e.location().rank().has_value());
        EXPECT_EQ(e.location().rank().value(), 0);
        EXPECT_EQ(e.required(), mpicxx::thread_support::MULTIPLE);
        EXPECT_EQ(e.provided(), mpicxx::thread_support::SINGLE);
    } catch (...) {
        FAIL() << "expected mpicxx::thread_support_not_satisfied exception";
    }
}