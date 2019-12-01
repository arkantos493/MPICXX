/**
 * @file info_env_test.cpp
 * @author Marcel Breyer
 * @date 2019-12-01
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the mpicxx::info::env object encapsulating the MPI_INFO_ENV information.
 */

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>

TEST(InfoTests, InfoEnv) {

    int nkeys_env;
    MPI_Info_get_nkeys(MPI_INFO_ENV, &nkeys_env);

    EXPECT_EQ(nkeys_env, mpicxx::info::env.size());

}