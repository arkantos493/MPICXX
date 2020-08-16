/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-16
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
 * | PredefinedErrorCategories                    | check the predefined MPI error categories                                                                                                                                    |
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


TEST(ErrorCategoryTest, PredefinedErrorCategories) {
    // check predefined MPI error categories
    EXPECT_EQ(mpicxx::error_category::success.value(), MPI_SUCCESS);
    EXPECT_EQ(mpicxx::error_category::buffer.value(), MPI_ERR_BUFFER);
    EXPECT_EQ(mpicxx::error_category::count.value(), MPI_ERR_COUNT);
    EXPECT_EQ(mpicxx::error_category::type.value(), MPI_ERR_TYPE);
    EXPECT_EQ(mpicxx::error_category::tag.value(), MPI_ERR_TAG);
    EXPECT_EQ(mpicxx::error_category::comm.value(), MPI_ERR_COMM);
    EXPECT_EQ(mpicxx::error_category::rank.value(), MPI_ERR_RANK);
    EXPECT_EQ(mpicxx::error_category::request.value(), MPI_ERR_REQUEST);
    EXPECT_EQ(mpicxx::error_category::root.value(), MPI_ERR_ROOT);
    EXPECT_EQ(mpicxx::error_category::group.value(), MPI_ERR_GROUP);
    EXPECT_EQ(mpicxx::error_category::op.value(), MPI_ERR_OP);
    EXPECT_EQ(mpicxx::error_category::topology.value(), MPI_ERR_TOPOLOGY);
    EXPECT_EQ(mpicxx::error_category::dims.value(), MPI_ERR_DIMS);
    EXPECT_EQ(mpicxx::error_category::arg.value(), MPI_ERR_ARG);
    EXPECT_EQ(mpicxx::error_category::unknown.value(), MPI_ERR_UNKNOWN);
    EXPECT_EQ(mpicxx::error_category::truncate.value(), MPI_ERR_TRUNCATE);
    EXPECT_EQ(mpicxx::error_category::other.value(), MPI_ERR_OTHER);
    EXPECT_EQ(mpicxx::error_category::intern.value(), MPI_ERR_INTERN);
    EXPECT_EQ(mpicxx::error_category::in_status.value(), MPI_ERR_IN_STATUS);
    EXPECT_EQ(mpicxx::error_category::pending.value(), MPI_ERR_PENDING);
    EXPECT_EQ(mpicxx::error_category::keyval.value(), MPI_ERR_KEYVAL);
    EXPECT_EQ(mpicxx::error_category::no_mem.value(), MPI_ERR_NO_MEM);
    EXPECT_EQ(mpicxx::error_category::base.value(), MPI_ERR_BASE);
    EXPECT_EQ(mpicxx::error_category::info_key.value(), MPI_ERR_INFO_KEY);
    EXPECT_EQ(mpicxx::error_category::info_value.value(), MPI_ERR_INFO_VALUE);
    EXPECT_EQ(mpicxx::error_category::info_nokey.value(), MPI_ERR_INFO_NOKEY);
    EXPECT_EQ(mpicxx::error_category::spawn.value(), MPI_ERR_SPAWN);
    EXPECT_EQ(mpicxx::error_category::port.value(), MPI_ERR_PORT);
    EXPECT_EQ(mpicxx::error_category::service.value(), MPI_ERR_SERVICE);
    EXPECT_EQ(mpicxx::error_category::name.value(), MPI_ERR_NAME);
    EXPECT_EQ(mpicxx::error_category::win.value(), MPI_ERR_WIN);
    EXPECT_EQ(mpicxx::error_category::size.value(), MPI_ERR_SIZE);
    EXPECT_EQ(mpicxx::error_category::disp.value(), MPI_ERR_DISP);
    EXPECT_EQ(mpicxx::error_category::info.value(), MPI_ERR_INFO);
    EXPECT_EQ(mpicxx::error_category::locktype.value(), MPI_ERR_LOCKTYPE);
    EXPECT_EQ(mpicxx::error_category::assert.value(), MPI_ERR_ASSERT);
    EXPECT_EQ(mpicxx::error_category::rma_conflict.value(), MPI_ERR_RMA_CONFLICT);
    EXPECT_EQ(mpicxx::error_category::rma_sync.value(), MPI_ERR_RMA_CONFLICT);
    EXPECT_EQ(mpicxx::error_category::rma_range.value(), MPI_ERR_RMA_RANGE);
    EXPECT_EQ(mpicxx::error_category::rma_attach.value(), MPI_ERR_RMA_ATTACH);
    EXPECT_EQ(mpicxx::error_category::rma_shared.value(), MPI_ERR_RMA_SHARED);
    EXPECT_EQ(mpicxx::error_category::rma_flavor.value(), MPI_ERR_RMA_FLAVOR);
    EXPECT_EQ(mpicxx::error_category::file.value(), MPI_ERR_FILE);
    EXPECT_EQ(mpicxx::error_category::not_same.value(), MPI_ERR_NOT_SAME);
    EXPECT_EQ(mpicxx::error_category::amode.value(), MPI_ERR_AMODE);
    EXPECT_EQ(mpicxx::error_category::unsupported_datarep.value(), MPI_ERR_UNSUPPORTED_DATAREP);
    EXPECT_EQ(mpicxx::error_category::unsupported_operation.value(), MPI_ERR_UNSUPPORTED_OPERATION);
    EXPECT_EQ(mpicxx::error_category::no_such_file.value(), MPI_ERR_NO_SUCH_FILE);
    EXPECT_EQ(mpicxx::error_category::file_exits.value(), MPI_ERR_FILE_EXISTS);
    EXPECT_EQ(mpicxx::error_category::bad_file.value(), MPI_ERR_BAD_FILE);
    EXPECT_EQ(mpicxx::error_category::access.value(), MPI_ERR_ACCESS);
    EXPECT_EQ(mpicxx::error_category::no_space.value(), MPI_ERR_NO_SPACE);
    EXPECT_EQ(mpicxx::error_category::quota.value(), MPI_ERR_QUOTA);
    EXPECT_EQ(mpicxx::error_category::read_only.value(), MPI_ERR_READ_ONLY);
    EXPECT_EQ(mpicxx::error_category::file_in_use.value(), MPI_ERR_FILE_IN_USE);
    EXPECT_EQ(mpicxx::error_category::dup_datarep.value(), MPI_ERR_DUP_DATAREP);
    EXPECT_EQ(mpicxx::error_category::conversion.value(), MPI_ERR_CONVERSION);
    EXPECT_EQ(mpicxx::error_category::io.value(), MPI_ERR_IO);
    EXPECT_EQ(mpicxx::error_category::lastcode.value(), MPI_ERR_LASTCODE);
}