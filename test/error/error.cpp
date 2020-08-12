/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-12
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the error classes.
 * @details Testsuite: *ErrorTest*
 * | test case name | test case description                         |
 * |:---------------|:----------------------------------------------|
 * |  |  |
 */

#include <mpicxx/error/error.hpp>

#include <gtest/gtest.h>

#include <iostream>


TEST(ErrorTest, AddNewErrorCode) {
    std::cout << std::boolalpha;

    mpicxx::error_category ca;
    std::vector<mpicxx::error_code> ecs = ca.add_error_code("CUSTOM ERROR CODE DESCRIPTION", "CUSTOM ERROR CODE DESCRIPTION 1");

    for (const mpicxx::error_code ec : ecs) {
        std::cout << ec << std::endl;
        std::cout << ec.value() << ": " << ec.message() << std::endl;
        std::cout << "Max: " << ec.last_used_value().value() << std::endl;
        std::cout << "Valid: " << static_cast<bool>(ec) << std::endl;
        std::cout << std::endl;
    }

}
