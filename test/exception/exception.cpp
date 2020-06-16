/**
 * @file test/exception/exception.cpp
 * @author Marcel Breyer
 * @date 2020-06-17
 *
 * @brief Test cases for the @ref mpicxx::exception class.
 * @details Testsuite: *ExceptionTest*
 * | test case name                       | test case description                                          |
 * |:-------------------------------------|:---------------------------------------------------------------|
 * | ThrowException                       | throw a base exception with source location information        |
 * | ThrowExceptionWithPrettyFunctionName | throw a base exception with better source location information |
 */

#include <gtest/gtest.h>

#include <mpicxx/exception/exception.hpp>


namespace {

    void function_that_throws() {
        throw mpicxx::exception();
    }

    void function_that_pretty_throws() {
        MPICXX_THROW_WITH_PRETTY_LOCATION(mpicxx::exception);
    }

}

TEST(ExceptionTest, ThrowException) {
    // exception should be thrown
    try {
        function_that_throws();
        FAIL() << "expected mpicxx::exception";
    } catch (const mpicxx::exception& e) {
        EXPECT_STREQ(e.location().file_name().c_str(), __FILE__);
        EXPECT_STREQ(e.location().function_name().c_str(), "function_that_throws");
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
        EXPECT_STREQ(e.location().file_name().c_str(), __FILE__);
        EXPECT_STREQ(e.location().function_name().c_str(), "void {anonymous}::function_that_pretty_throws()");
        ASSERT_TRUE(e.location().rank().has_value());
        EXPECT_EQ(e.location().rank().value(), 0);
    } catch (...) {
        FAIL() << "expected mpicxx::exception";
    }
}