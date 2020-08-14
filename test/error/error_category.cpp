/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-14
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::error_category class.
 * @details Testsuite: *ErrorCategoryTest*
 * | test case name                               | test case description                                                                                                                                                        |
 * |:---------------------------------------------|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * | CreateNewErrorCategory                       | create new error categories                                                                                                                                                  |
 * | AddErrorCodeByStringView                     | add a new error code                                                                                                                                                         |
 * | AddErrorCodeByStringViewInvalidCategory      | try to add a new error code to an error category with invalid value (death test)                                                                                             |
 * | AddErrorCodeByInvalidStringView              | try to add a new error code with an illegal error string (death test)                                                                                                        |
 * | AddErrorCodeByIteratorRange                  | add all error codes in the given iterator range                                                                                                                              |
 * | AddErrorCodeByInvalidIteratorRange           | try to add all error codes in the given, illegal iterator range (death test)                                                                                                 |
 * | AddErrorCodeByIteratorRangeInvalidCategory   | try to add all error codes in the given iterator range to an error category with invalid value (death test)                                                                  |
 * | AddErrorCodeByIteratorRangeInvalidValue      | try to add all error codes with an illegal error string in the given iterator range (death test)                                                                             |
 * | AddErrorCodeByInitializerList                | add all error codes in the given [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list)                                                       |
 * | AddErrorCodeByInitializerListInvalidCategory | try to add all error codes in the [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) to an error category with invalid value (death test) |
 * | AddErrorCodeByInitializerListInvalidValue    | try to add all error codes with an illegal error string in the [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) (death test)            |
 * | AddErrorCodeByParameterPack                  | add all error codes in the given parameter pack                                                                                                                              |
 * | AddErrorCodeByParameterPackInvalidCategory   | try to add all error codes in the given parameter pack to an error category with invalid value (death test)                                                                  |
 * | AddErrorCodeByParameterPackInvalidValue      | try to add all error codes with an illegal error string in the parameter pack (death test)                                                                                   |
 * | ErrorCategoryGetValue                        | get the current error category value                                                                                                                                         |
 * | ErrorCategoryThreeWayComparison              | check if the comparison operators `==`, `!=`, `<`, `<=`, `>` and `>=` work                                                                                                   |
 * | ErrorCategoryStreamInsertionOperator         | check if outputting an error code works as intended: `err_category_value`                                                                                                    |
 */

#include <mpicxx/error/error.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <initializer_list>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

TEST(ErrorCategoryTest, CreateNewErrorCategory) {
    // create new error category
    mpicxx::error_category new_category;

    // the error category value should be valid
    EXPECT_GE(new_category.value(), MPI_SUCCESS);
}

TEST(ErrorCategoryTest, AddErrorCodeByStringView) {
    // create new error category
    mpicxx::error_category new_category;

    // add error codes to error category
    mpicxx::error_code ec1 = new_category.add_error_code("ERROR_STRING: one");
    mpicxx::error_code ec2 = new_category.add_error_code("ERROR_STRING: two");
    mpicxx::error_code ec3 = new_category.add_error_code("");

    // check that the new error codes are associated to the correct error category
    EXPECT_EQ(ec1.category().value(), new_category.value());
    EXPECT_EQ(ec2.category().value(), new_category.value());
    EXPECT_EQ(ec3.category().value(), new_category.value());

    // check whether error strings where set correctly
    EXPECT_STREQ(ec1.message().c_str(), "ERROR_STRING: one");
    EXPECT_STREQ(ec2.message().c_str(), "ERROR_STRING: two");
    EXPECT_STREQ(ec3.message().c_str(), "");
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByStringViewInvalidCategory) {
    // create new error category and set illegal value
    mpicxx::error_category new_category;
    *reinterpret_cast<int*>(&new_category) = -1;

    // try to add error code to error category with invalid value
    [[maybe_unused]] mpicxx::error_code ec;
    ASSERT_DEATH( ec = new_category.add_error_code("ERROR_STRING") , "");
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByInvalidStringView) {
    // create new error category
    mpicxx::error_category new_category;

    // try to add error code with illegal error string to error category
    const std::string max_error_string(MPI_MAX_ERROR_STRING, 'x');
    [[maybe_unused]] mpicxx::error_code ec;
    ASSERT_DEATH( ec = new_category.add_error_code(max_error_string.c_str()) , "");
}

TEST(ErrorCategoryTest, AddErrorCodeByIteratorRange) {
    // create new error category
    mpicxx::error_category new_category;

    // add error codes to error category
    std::vector<std::string> vec = { "ERROR_STRING: one", "ERROR_STRING: two", "ERROR_STRING: three" };
    std::vector<mpicxx::error_code> ecs = new_category.add_error_code(vec.begin(), vec.end());

    // check that the new error codes are associated to the correct error category
    // and check whether error strings where set correctly
    ASSERT_EQ(vec.size(), ecs.size());
    for (std::size_t i = 0; i < vec.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ecs[i].category().value(), new_category.value());
        EXPECT_EQ(ecs[i].message(), vec[i]);
    }
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByInvalidIteratorRange) {
    // create new error category
    mpicxx::error_category new_category;

    // try to add error codes to error category using an invalid iterator range
    std::vector<std::string> vec = { "ERROR_STRING: one", "ERROR_STRING: two", "ERROR_STRING: three" };
    [[maybe_unused]] std::vector<mpicxx::error_code> ecs;
    ASSERT_DEATH( ecs = new_category.add_error_code(vec.end(), vec.begin()) , "");
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByIteratorRangeInvalidCategory) {
    // create new error category and set illegal value
    mpicxx::error_category new_category;
    *reinterpret_cast<int*>(&new_category) = -1;

    // try to add error code to error category with invalid value
    std::vector<std::string> vec = { "ERROR_STRING" };
    [[maybe_unused]] std::vector<mpicxx::error_code> ec;
    ASSERT_DEATH( ec = new_category.add_error_code(vec.begin(), vec.end()) , "");
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByIteratorRangeInvalidValue) {
    // create new error category
    mpicxx::error_category new_category;

    // try to add error code with illegal error string to error category
    std::vector<std::string> vec = { std::string(MPI_MAX_ERROR_STRING, 'x') };
    [[maybe_unused]] std::vector<mpicxx::error_code> ecs;
    ASSERT_DEATH( ecs = new_category.add_error_code(vec.begin(), vec.end()) , "");
}

TEST(ErrorCategoryTest, AddErrorCodeByInitializerList) {
    // create new error category
    mpicxx::error_category new_category;

    // add error codes to error category
    std::initializer_list<std::string> ilist = { "ERROR_STRING: one", "ERROR_STRING: two", "ERROR_STRING: three" };
    std::vector<mpicxx::error_code> ecs = new_category.add_error_code(ilist);

    // check that the new error codes are associated to the correct error category
    // and check whether error strings where set correctly
    ASSERT_EQ(ilist.size(), ecs.size());
    for (std::size_t i = 0; i < ilist.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ecs[i].category().value(), new_category.value());
        EXPECT_EQ(ecs[i].message(), *(ilist.begin() + i));
    }
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByInitializerListInvalidCategory) {
    // create new error category and set illegal value
    mpicxx::error_category new_category;
    *reinterpret_cast<int*>(&new_category) = -1;

    // try to add error code to error category with invalid value
    std::initializer_list<std::string> ilist = { "ERROR_STRING" };
    [[maybe_unused]] std::vector<mpicxx::error_code> ec;
    ASSERT_DEATH( ec = new_category.add_error_code(ilist) , "");
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByInitializerListInvalidValue) {
    // create new error category
    mpicxx::error_category new_category;

    // try to add error code with illegal error string to error category
    std::initializer_list<std::string> ilist = { std::string(MPI_MAX_ERROR_STRING, 'x') };
    [[maybe_unused]] std::vector<mpicxx::error_code> ecs;
    ASSERT_DEATH( ecs = new_category.add_error_code(ilist) , "");
}

TEST(ErrorCategoryTest, AddErrorCodeByParameterPack) {
    // create new error category
    mpicxx::error_category new_category;

    // add error codes to error category
    std::vector<std::string> vec = { "ERROR_STRING: one", "ERROR_STRING: two", "ERROR_STRING: three" };
    std::vector<mpicxx::error_code> ecs = new_category.add_error_code("ERROR_STRING: one", "ERROR_STRING: two", "ERROR_STRING: three");

    // check that the new error codes are associated to the correct error category
    // and check whether error strings where set correctly
    ASSERT_EQ(vec.size(), ecs.size());
    for (std::size_t i = 0; i < vec.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(ecs[i].category().value(), new_category.value());
        EXPECT_EQ(ecs[i].message(), vec[i]);
    }
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByParameterPackInvalidCategory) {
    // create new error category and set illegal value
    mpicxx::error_category new_category;
    *reinterpret_cast<int*>(&new_category) = -1;

    // try to add error code to error category with invalid value
    [[maybe_unused]] std::vector<mpicxx::error_code> ec;
    ASSERT_DEATH( ec = new_category.add_error_code("ERROR_STRING: one", "ERROR_STRING: two") , "");
}

TEST(ErrorCategoryDeathTest, AddErrorCodeByParameterPackInvalidValue) {
    // create new error category
    mpicxx::error_category new_category;

    // try to add error code with illegal error string to error category
    [[maybe_unused]] std::vector<mpicxx::error_code> ecs;
    ASSERT_DEATH( ecs = new_category.add_error_code("ERROR_STRING", std::string(MPI_MAX_ERROR_STRING, 'x')) , "");
}


TEST(ErrorCategoryTest, ErrorCategoryGetValue) {
    // create new valid error codes
    mpicxx::error_category cat1 = mpicxx::error_code{}.category();
    mpicxx::error_category cat2 = mpicxx::error_code{1}.category();

    // check getter for correctness
    EXPECT_EQ(cat1.value(), MPI_SUCCESS);
    EXPECT_EQ(cat2.value(), 1);
}


TEST(ErrorCategoryTest, ErrorCategoryThreeWayComparison) {
    // create valid error categories
    mpicxx::error_category ec0 = mpicxx::error_code(0).category();
    mpicxx::error_category ec1 = mpicxx::error_code(1).category();
    mpicxx::error_category ec2 = mpicxx::error_code(2).category();

    // equality
    EXPECT_TRUE(ec0 == ec0);
    EXPECT_FALSE(ec0 == ec1);
    EXPECT_FALSE(ec2 == ec0);

    // inequality
    EXPECT_FALSE(ec0 != ec0);
    EXPECT_TRUE(ec0 != ec1);
    EXPECT_TRUE(ec2 != ec0);

    // less than
    EXPECT_FALSE(ec0 < ec0);
    EXPECT_TRUE(ec0 < ec1);
    EXPECT_FALSE(ec2 < ec0);

    // less or equal than
    EXPECT_TRUE(ec0 <= ec0);
    EXPECT_TRUE(ec0 <= ec1);
    EXPECT_FALSE(ec2 <= ec0);

    // greater than
    EXPECT_FALSE(ec0 > ec0);
    EXPECT_FALSE(ec0 > ec1);
    EXPECT_TRUE(ec2 > ec0);

    // greater or equal than
    EXPECT_TRUE(ec0 >= ec0);
    EXPECT_FALSE(ec0 >= ec1);
    EXPECT_TRUE(ec2 >= ec0);
}


TEST(ErrorCategoryTest, ErrorCategoryStreamInsertionOperator) {
    // create new error category
    mpicxx::error_category ec = mpicxx::error_code(1).category();

    // serialize error category
    std::stringstream ss;
    ss << ec;

    // compare strings
    EXPECT_STREQ(ss.str().c_str(), "1");
}