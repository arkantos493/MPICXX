/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-19
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::error_handler class.
 * @details Testsuite: *ErrorHandlerTest*
 * | test case name | test case description |
 * |:---------------|:----------------------|
 * |                |                       |
 */

#include <mpicxx/error/error_handler.hpp>
#include <mpicxx/error/error.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

#include <iostream>

void exception_fn(const mpicxx::error_code ec) {
    std::cout << ec.message() << std::endl;
    MPI_Abort(MPI_COMM_WORLD, ec.value());
}

TEST(ErrorHandlerTest, Tests) {

    std::cout << static_cast<int>(mpicxx::error_handler::type::comm) << " " << mpicxx::error_handler::type::comm << std::endl;
    std::cout << static_cast<int>(mpicxx::error_handler::type::file) << " " << mpicxx::error_handler::type::file << std::endl;
    std::cout << static_cast<int>(mpicxx::error_handler::type::win)  << " " << mpicxx::error_handler::type::win  << std::endl;

    mpicxx::error_handler handler = mpicxx::make_error_handler<exception_fn>(mpicxx::error_handler::type::comm);

    MPI_Comm_set_errhandler(MPI_COMM_WORLD, handler.handler_[0]);
//    MPI_Comm_call_errhandler(MPI_COMM_WORLD, 42);
}
