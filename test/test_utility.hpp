/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-28
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Utility functions used in multiple test files.
 */

#ifndef MPICXX_TEST_UTILITY_HPP
#define MPICXX_TEST_UTILITY_HPP

#include <exception>
#include <string_view>

#include <gtest/gtest.h>

/**
 * @def EXPECT_THROW_WHAT
 * @brief Defines a macro like
 *        <a href="https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md">googletest</a>'s
 *        `EXPECT_THROW`, but also allows to test for the correct exception's
 *        [`what()`](https://en.cppreference.com/w/cpp/error/exception/what) message.
 *
 * @param[in] statement the statement which should throw (a specific exception)
 * @param[in] expected_exception the type of the exception which should get thrown
 * @param[in] msg the expected exception's [`what()`](https://en.cppreference.com/w/cpp/error/exception/what) message
 */
#define EXPECT_THROW_WHAT(statement, expected_exception, msg)         \
do {                                                                  \
    try {                                                             \
        statement;                                                    \
        FAIL() << "Expected " #expected_exception;                    \
    } catch (const expected_exception& e) {                           \
        EXPECT_EQ(std::string_view(e.what()), std::string_view(msg)); \
    } catch(...) {                                                    \
        FAIL() << "Expected " #expected_exception;                    \
    }                                                                 \
} while(false);                                                       \

#endif // MPICXX_TEST_UTILITY_HPP