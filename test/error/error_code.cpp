/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-16
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::error_code class.
 * @details Testsuite: *ErrorCodeTest*
 * | test case name                          | test case description                                                                                                                                           |
 * |:----------------------------------------|:----------------------------------------------------------------------------------------------------------------------------------------------------------------|
 * | CreateNewErrorCode                      | create new error codes                                                                                                                                          |
 * | CreateNewIllegalErrorCode               | try creating new error codes with illegal values                                                                                                                |
 * | AssignToErrorCode                       | replace the current error code value with a new one                                                                                                             |
 * | AssignInvalidToErrorCode                | try replacing the current error code value with a new illegal one (death test)                                                                                  |
 * | ClearErrorCode                          | reset the current error code value to [*MPI_SUCCESS*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node222.htm)                                          |
 * | ErrorCodeGetValue                       | get the current error code value                                                                                                                                |
 * | ErrorCodeGetLastUsedCode                | get the last used error code (value)                                                                                                                            |
 * | ErrorCodeGetCategory                    | check for the correct error category                                                                                                                            |
 * | InvalidErrorCodeGetCategory             | try retrieving an error category from an error code with illegal value (death test)                                                                             |
 * | ErrorCodeGetMessage                     | check for the correct error code messages                                                                                                                       |
 * | InvalidErrorCodeGetMessage              | try retrieving an error code message from an error code with illegal value (death test)                                                                         |
 * | ErrorCodeOperatorBool                   | check if `static_cast<bool>(ec)` returns `false` if and only if `ec` refers to [*MPI_SUCCESS*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node222.htm) |
 * | ErrorCodeThreeWayComparison             | check if the comparison operators `==`, `!=`, `<`, `<=`, `>` and `>=` work                                                                                      |
 * | ErrorCodeStreamInsertionOperator        | check if outputting an error code works as intended: `err_code_value: error_code_string`                                                                        |
 * | InvalidErrorCodeStreamInsertionOperator | try to output an error code with illegal value (death test)                                                                                                     |
 * | HashErrorCode                           | check if the hash of an @ref mpicxx::error_code is the same as the hash of the corresponding int                                                                |
 * | PredefinedErrorCodes                    | check the predefined MPI error codes                                                                                                                            |
 */

#include <mpicxx/error/error.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <functional>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

TEST(ErrorCodeTest, CreateNewErrorCode) {
    // create new valid error codes
    mpicxx::error_code ec1 = 1;
    EXPECT_EQ(ec1.value(), 1);

    mpicxx::error_code ec2 = 3;
    EXPECT_EQ(ec2.value(), 3);
    EXPECT_EQ(*reinterpret_cast<int*>(&ec2), 3);

    // default constructed error code equals MPI_SUCCESS
    mpicxx::error_code ec3;
    EXPECT_EQ(ec3.value(), MPI_SUCCESS);
}

TEST(ErrorCodeDeathTest, CreateNewIllegalErrorCode) {
    // create new invalid error codes
    ASSERT_DEATH( [[maybe_unused]] mpicxx::error_code ec1(-1) , "");
    ASSERT_DEATH( [[maybe_unused]] mpicxx::error_code ec2 = std::numeric_limits<int>::max() , "");
}


TEST(ErrorCodeTest, AssignToErrorCode) {
    // create new valid error code and check for correctness
    mpicxx::error_code ec(5);
    ASSERT_EQ(ec.value(), 5);

    // assign new value and check for correctness
    ec.assign(3);
    EXPECT_EQ(ec.value(), 3);
}

TEST(ErrorCodeDeathTest, AssignInvalidToErrorCode) {
    // create new valid error code and check for correctness
    mpicxx::error_code ec(5);
    ASSERT_EQ(ec.value(), 5);

    // try to assign a new invalid value and check for correctness
    ASSERT_DEATH( ec.assign(-1) , "");
}


TEST(ErrorCodeTest, ClearErrorCode) {
    // create new valid error code and check for correctness
    mpicxx::error_code ec(5);
    ASSERT_EQ(ec.value(), 5);

    // clear error code value and check for correctness
    ec.clear();
    EXPECT_EQ(ec.value(), MPI_SUCCESS);
}


TEST(ErrorCodeTest, ErrorCodeGetValue) {
    // create new valid error codes
    mpicxx::error_code ec1;
    mpicxx::error_code ec2(1);

    // check getter for correctness
    EXPECT_EQ(ec1.value(), MPI_SUCCESS);
    EXPECT_EQ(ec2.value(), 1);
}


TEST(ErrorCodeTest, ErrorCodeGetLastUsedCode) {
    // get last used error code
    std::optional<int> last_error_code = mpicxx::error_code::last_used_value();
    if (last_error_code.has_value()) {
        EXPECT_GT(last_error_code.value(), 0);
    }
}


TEST(ErrorCodeTest, ErrorCodeGetCategory) {
    // create new valid error code
    mpicxx::error_code ec(2);

    // get error category
    mpicxx::error_category eca = ec.category();

    // check error category: default MPI error codes are mapped to error categories 1:1
    EXPECT_EQ(eca.value(), ec.value());
    int category;
    MPI_Error_class(ec.value(), &category);
    EXPECT_EQ(ec.value(), category);
}

TEST(ErrorCodeDeathTest, InvalidErrorCodeGetCategory) {
    // create error code and set invalid value
    mpicxx::error_code ec;
    *reinterpret_cast<int*>(&ec) = -1;
    ASSERT_EQ(ec.value(), -1);

    // try to get error category for an invalid error code value
    ASSERT_DEATH( [[maybe_unused]] mpicxx::error_category cat = ec.category() , "");
}


TEST(ErrorCodeTest, ErrorCodeGetMessage) {
    // create new error code
    mpicxx::error_code ec;

    // get error string and check for validity
    std::string es = ec.message();
    ASSERT_FALSE(es.empty());

    char error_string[MPI_MAX_ERROR_STRING];
    int resultlen;
    MPI_Error_string(ec.value(), error_string, &resultlen);
    EXPECT_STREQ(error_string, es.c_str());
}

TEST(ErrorCodeDeathTest, InvalidErrorCodeGetMessage) {
    // create error code and set invalid value
    mpicxx::error_code ec;
    *reinterpret_cast<int*>(&ec) = -1;
    ASSERT_EQ(ec.value(), -1);

    // try to get error string for an invalid error code value
    [[maybe_unused]] std::string str;
    ASSERT_DEATH( str =  ec.message() , "");
}

TEST(ErrorCodeTest, ErrorCodeGetMaxErrorStringSize) {
    // check maximum error string size
    EXPECT_EQ(mpicxx::error_code::max_message_size(), MPI_MAX_ERROR_STRING);
}


TEST(ErrorCodeTest, ErrorCodeOperatorBool) {
    // create new valid error codes and check for validity
    mpicxx::error_code ec1 = 1;
    EXPECT_TRUE(static_cast<bool>(ec1));

    mpicxx::error_code ec2 = 3;
    EXPECT_TRUE(static_cast<bool>(ec2));

    // default constructed error code equals MPI_SUCCESS and check for validity
    mpicxx::error_code ec3;
    EXPECT_FALSE(static_cast<bool>(ec3));
}


TEST(ErrorCodeTest, ErrorCodeThreeWayComparison) {
    // create valid error codes
    mpicxx::error_code ec0(0);
    mpicxx::error_code ec1(1);
    mpicxx::error_code ec2(2);

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


TEST(ErrorCodeTest, ErrorCodeStreamInsertionOperator) {
    // create new error code
    mpicxx::error_code ec(1);

    // serialize error code
    std::stringstream ss1;
    ss1 << ec;

    // calculate correct error code serialization
    std::stringstream ss2;
    char error_string[MPI_MAX_ERROR_STRING];
    int resultlen;
    MPI_Error_string(1, error_string, &resultlen);
    ss2 << 1 << ": " << std::string_view(error_string, resultlen);

    // compare strings
    EXPECT_EQ(ss1.str(), ss2.str());
}

TEST(ErrorCodeDeathTest, InvalidErrorCodeStreamInsertionOperator) {
    // create error code and set invalid value
    mpicxx::error_code ec;
    *reinterpret_cast<int*>(&ec) = -1;
    ASSERT_EQ(ec.value(), -1);

    // try to serialize error code
    std::stringstream ss1;
    ASSERT_DEATH( ss1 << ec , "");
}


TEST(ErrorCodeTest, HashErrorCode) {
    // create error code
    mpicxx::error_code ec0(0);
    mpicxx::error_code ec1(2);

    // check if hashes are equal
    EXPECT_EQ(std::hash<mpicxx::error_code>{}(ec0), std::hash<int>{}(0));
    EXPECT_EQ(std::hash<mpicxx::error_code>{}(ec1), std::hash<int>{}(2));
}


TEST(ErrorCodeTest, PredefinedErrorCodes) {
    // check predefined MPI error codes
    EXPECT_EQ(mpicxx::error_code::success.value(), MPI_SUCCESS);
    EXPECT_EQ(mpicxx::error_code::buffer.value(), MPI_ERR_BUFFER);
    EXPECT_EQ(mpicxx::error_code::count.value(), MPI_ERR_COUNT);
    EXPECT_EQ(mpicxx::error_code::type.value(), MPI_ERR_TYPE);
    EXPECT_EQ(mpicxx::error_code::tag.value(), MPI_ERR_TAG);
    EXPECT_EQ(mpicxx::error_code::comm.value(), MPI_ERR_COMM);
    EXPECT_EQ(mpicxx::error_code::rank.value(), MPI_ERR_RANK);
    EXPECT_EQ(mpicxx::error_code::request.value(), MPI_ERR_REQUEST);
    EXPECT_EQ(mpicxx::error_code::root.value(), MPI_ERR_ROOT);
    EXPECT_EQ(mpicxx::error_code::group.value(), MPI_ERR_GROUP);
    EXPECT_EQ(mpicxx::error_code::op.value(), MPI_ERR_OP);
    EXPECT_EQ(mpicxx::error_code::topology.value(), MPI_ERR_TOPOLOGY);
    EXPECT_EQ(mpicxx::error_code::dims.value(), MPI_ERR_DIMS);
    EXPECT_EQ(mpicxx::error_code::arg.value(), MPI_ERR_ARG);
    EXPECT_EQ(mpicxx::error_code::unknown.value(), MPI_ERR_UNKNOWN);
    EXPECT_EQ(mpicxx::error_code::truncate.value(), MPI_ERR_TRUNCATE);
    EXPECT_EQ(mpicxx::error_code::other.value(), MPI_ERR_OTHER);
    EXPECT_EQ(mpicxx::error_code::intern.value(), MPI_ERR_INTERN);
    EXPECT_EQ(mpicxx::error_code::in_status.value(), MPI_ERR_IN_STATUS);
    EXPECT_EQ(mpicxx::error_code::pending.value(), MPI_ERR_PENDING);
    EXPECT_EQ(mpicxx::error_code::keyval.value(), MPI_ERR_KEYVAL);
    EXPECT_EQ(mpicxx::error_code::no_mem.value(), MPI_ERR_NO_MEM);
    EXPECT_EQ(mpicxx::error_code::base.value(), MPI_ERR_BASE);
    EXPECT_EQ(mpicxx::error_code::info_key.value(), MPI_ERR_INFO_KEY);
    EXPECT_EQ(mpicxx::error_code::info_value.value(), MPI_ERR_INFO_VALUE);
    EXPECT_EQ(mpicxx::error_code::info_nokey.value(), MPI_ERR_INFO_NOKEY);
    EXPECT_EQ(mpicxx::error_code::spawn.value(), MPI_ERR_SPAWN);
    EXPECT_EQ(mpicxx::error_code::port.value(), MPI_ERR_PORT);
    EXPECT_EQ(mpicxx::error_code::service.value(), MPI_ERR_SERVICE);
    EXPECT_EQ(mpicxx::error_code::name.value(), MPI_ERR_NAME);
    EXPECT_EQ(mpicxx::error_code::win.value(), MPI_ERR_WIN);
    EXPECT_EQ(mpicxx::error_code::size.value(), MPI_ERR_SIZE);
    EXPECT_EQ(mpicxx::error_code::disp.value(), MPI_ERR_DISP);
    EXPECT_EQ(mpicxx::error_code::info.value(), MPI_ERR_INFO);
    EXPECT_EQ(mpicxx::error_code::locktype.value(), MPI_ERR_LOCKTYPE);
    EXPECT_EQ(mpicxx::error_code::assert.value(), MPI_ERR_ASSERT);
    EXPECT_EQ(mpicxx::error_code::rma_conflict.value(), MPI_ERR_RMA_CONFLICT);
    EXPECT_EQ(mpicxx::error_code::rma_sync.value(), MPI_ERR_RMA_CONFLICT);
    EXPECT_EQ(mpicxx::error_code::rma_range.value(), MPI_ERR_RMA_RANGE);
    EXPECT_EQ(mpicxx::error_code::rma_attach.value(), MPI_ERR_RMA_ATTACH);
    EXPECT_EQ(mpicxx::error_code::rma_shared.value(), MPI_ERR_RMA_SHARED);
    EXPECT_EQ(mpicxx::error_code::rma_flavor.value(), MPI_ERR_RMA_FLAVOR);
    EXPECT_EQ(mpicxx::error_code::file.value(), MPI_ERR_FILE);
    EXPECT_EQ(mpicxx::error_code::not_same.value(), MPI_ERR_NOT_SAME);
    EXPECT_EQ(mpicxx::error_code::amode.value(), MPI_ERR_AMODE);
    EXPECT_EQ(mpicxx::error_code::unsupported_datarep.value(), MPI_ERR_UNSUPPORTED_DATAREP);
    EXPECT_EQ(mpicxx::error_code::unsupported_operation.value(), MPI_ERR_UNSUPPORTED_OPERATION);
    EXPECT_EQ(mpicxx::error_code::no_such_file.value(), MPI_ERR_NO_SUCH_FILE);
    EXPECT_EQ(mpicxx::error_code::file_exits.value(), MPI_ERR_FILE_EXISTS);
    EXPECT_EQ(mpicxx::error_code::bad_file.value(), MPI_ERR_BAD_FILE);
    EXPECT_EQ(mpicxx::error_code::access.value(), MPI_ERR_ACCESS);
    EXPECT_EQ(mpicxx::error_code::no_space.value(), MPI_ERR_NO_SPACE);
    EXPECT_EQ(mpicxx::error_code::quota.value(), MPI_ERR_QUOTA);
    EXPECT_EQ(mpicxx::error_code::read_only.value(), MPI_ERR_READ_ONLY);
    EXPECT_EQ(mpicxx::error_code::file_in_use.value(), MPI_ERR_FILE_IN_USE);
    EXPECT_EQ(mpicxx::error_code::dup_datarep.value(), MPI_ERR_DUP_DATAREP);
    EXPECT_EQ(mpicxx::error_code::conversion.value(), MPI_ERR_CONVERSION);
    EXPECT_EQ(mpicxx::error_code::io.value(), MPI_ERR_IO);
    EXPECT_EQ(mpicxx::error_code::lastcode.value(), MPI_ERR_LASTCODE);
}