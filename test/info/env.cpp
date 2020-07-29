/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-29
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Test cases for the @ref mpicxx::info::env static const info object provided by the @ref mpicxx::info class.
 * @details Testsuite: *InfoEnvTest*
 * | test case name | test case description                                                                                    |
 * |:---------------|:---------------------------------------------------------------------------------------------------------|
 * | InfoEnv        | check elements against [*MPI_INFO_ENV*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) |
 */

#include <mpicxx/info/info.hpp>

#include <gtest/gtest.h>
#include <mpi.h>

TEST(InfoEnvTest, InfoEnv) {
    // check whether the same amount of keys a present
    int nkeys_env, nkeys;
    MPI_Info_get_nkeys(MPI_INFO_ENV, &nkeys_env);
    MPI_Info_get_nkeys(mpicxx::info::env.get(), &nkeys);
    ASSERT_EQ(nkeys_env, nkeys);

    // check if all [key, value]-pairs are equivalent
    char key_env[MPI_MAX_INFO_KEY];
    char key[MPI_MAX_INFO_KEY];
    char value_env[MPI_MAX_INFO_VAL];
    char value[MPI_MAX_INFO_VAL];
    for (int i = 0; i < nkeys; ++i) {
        SCOPED_TRACE(i);
        // get keys
        MPI_Info_get_nthkey(MPI_INFO_ENV, i, key_env);
        MPI_Info_get_nthkey(mpicxx::info::env.get(), i, key);
        ASSERT_STREQ(key_env, key);

        // get value length
        int valuelen_env, valuelen, flag;
        MPI_Info_get_valuelen(MPI_INFO_ENV, key_env, &valuelen_env, &flag);
        MPI_Info_get_valuelen(mpicxx::info::env.get(), key, &valuelen, &flag);
        ASSERT_EQ(valuelen_env, valuelen);

        // get value
        MPI_Info_get(MPI_INFO_ENV, key_env, valuelen_env, value_env, &flag);
        MPI_Info_get(mpicxx::info::env.get(), key_env, valuelen, value, &flag);
        ASSERT_STREQ(value_env, value);
    }
}